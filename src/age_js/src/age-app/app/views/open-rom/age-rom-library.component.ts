//
// Copyright 2019 Christoph Sprenger
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

import {HttpClient} from '@angular/common/http';
import {ChangeDetectionStrategy, Component, Input, OnInit} from '@angular/core';
import {AgeIconsService} from 'age-lib';
import {BehaviorSubject, combineLatest, Observable} from 'rxjs';
import {map} from 'rxjs/operators';
import {AgeNavigationService} from '../../common';


@Component({
    selector: 'age-rom-library',
    template: `
        <ng-container *ngIf="(filteredRoms$ | async) as filteredRoms">

            <mat-card *ngFor="let rom of filteredRoms; trackBy: trackByTitle">

                <a class="age-ui-clickable"
                   [routerLink]="rom.loadRomRouterLink"
                   [title]="rom.clickTooltip">
                    <img mat-card-image
                         [alt]="rom.screenshotAlt"
                         [src]="rom.romScreenshotUrl">
                </a>

                <mat-card-title>{{rom.romTitle}}</mat-card-title>

                <mat-card-subtitle>
                    {{rom.romType}}<br>
                    by
                    <a *ngIf="rom.romAuthorsUrl"
                       class="age-ui-clickable"
                       [age-href]="rom.romAuthorsUrl"
                       [title]="rom.authorsLinkTooltip">
                        {{rom.romAuthors}}
                    </a>
                    <ng-container *ngIf="!rom.romAuthorsUrl">{{rom.romAuthors}}</ng-container>
                </mat-card-subtitle>

                <mat-card-actions>

                    <a mat-button
                       [routerLink]="rom.loadRomRouterLink"
                       [title]="rom.clickTooltip">
                        <mat-icon [svgIcon]="icons.faPlay"></mat-icon>
                    </a>

                    <a mat-button
                       [age-href]="rom.romSiteUrl"
                       [title]="rom.romSiteLinkTooltip">
                        <mat-icon [svgIcon]="icons.faHome"></mat-icon>
                    </a>

                    <a *ngIf="rom.romSourceUrl as romSourceUrl"
                       mat-button
                       [age-href]="romSourceUrl"
                       [title]="rom.romSourceLinkTooltip">
                        <mat-icon [svgIcon]="rom.romSourceIconName"></mat-icon>
                    </a>

                </mat-card-actions>

            </mat-card>

        </ng-container>
    `,
    styles: [`
        :host {
            display: grid;
            justify-content: center;
            grid-gap: 16px 16px;
            /* width required for 3 mat-card-actions */
            grid-template-columns: repeat(auto-fit, 240px);
        }

        mat-card-actions {
            white-space: nowrap;
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeRomLibraryComponent implements OnInit {

    private readonly _filterSubject = new BehaviorSubject<string>('');
    private _filteredRoms$?: Observable<ReadonlyArray<IAgeRomLibraryItem>>;

    constructor(readonly icons: AgeIconsService,
                readonly navigationService: AgeNavigationService,
                private readonly _httpClient: HttpClient) {
    }

    ngOnInit(): void {
        const roms$: Observable<IAgeRomLibraryItem[]> = this._httpClient
            .get(
                'assets/rom-library.json',
                {
                    observe: 'body',
                    responseType: 'json',
                },
            )
            .pipe(
                map(romLibraryJson => (romLibraryJson as IAgeRomLibraryContents).roms),
                map(roms => roms.slice().sort(
                    (aRom: IAgeOnlineRom, bRom: IAgeOnlineRom) => aRom.romTitle.localeCompare(bRom.romTitle),
                )),
                map(roms => roms.map(rom => {
                    const filterString = newFilterString(
                        ...rom.romTypes,
                        rom.romTitle,
                        rom.romAuthors,
                    );
                    return {
                        ...rom,
                        filterString,
                        romType: rom.romTypes.join(', '),
                        screenshotAlt: `${rom.romTitle} screenshot`,
                        loadRomRouterLink: this.navigationService.romUrlRouterLink(rom.romFileUrl),
                        clickTooltip: `run ${rom.romTitle}`,
                        authorsLinkTooltip: `${rom.romAuthors} on pouet.net`,
                        romSiteLinkTooltip: `${rom.romTitle} on pouet.net`,
                        romSourceLinkTooltip: `${rom.romTitle} source code`,
                        romSourceIconName: this._sourceLinkIconName(rom.romSourceUrl),
                    };
                })),
            );

        this._filteredRoms$ = combineLatest([
            roms$,
            this._filterSubject.asObservable(),
        ]).pipe(
            map(values => {
                const roms = values[0];
                const filter = values[1];

                return !filter
                    ? roms
                    : roms.filter(rom => rom.filterString.indexOf(filter.toLocaleLowerCase()) >= 0);
            }),
        );
    }


    get filteredRoms$(): Observable<ReadonlyArray<IAgeRomLibraryItem>> | undefined {
        return this._filteredRoms$;
    }

    @Input() set filter(filter: string | null | undefined) {
        this._filterSubject.next(filter || '');
    }


    trackByTitle(_index: number, libraryItem: IAgeRomLibraryItem): string {
        return libraryItem.romTitle;
    }

    private _sourceLinkIconName(url?: string): string | undefined {
        if (!url) {
            return undefined;
        }
        switch (new URL(url).hostname) {

            case 'github.com':
                return this.icons.faGithub;

            case 'gitlab.com':
                return this.icons.faGitlab;

            default:
                return this.icons.faFileCode;
        }
    }
}


interface IAgeRomLibraryContents {
    readonly roms: ReadonlyArray<IAgeOnlineRom>;
}

type TAgeRomType = 'demo' | 'game' | 'intro' | 'invitation';

interface IAgeOnlineRom {
    readonly romTypes: ReadonlyArray<TAgeRomType>;
    readonly romTitle: string; // this is the primary key
    readonly romAuthors: string;
    readonly romFileUrl: string;
    readonly romScreenshotUrl: string;
    readonly romAuthorsUrl?: string;
    readonly romSiteUrl: string;
    readonly romSourceUrl?: string;
}

interface IAgeRomLibraryItem extends IAgeOnlineRom {
    readonly filterString: string;
    readonly romType: string;
    readonly screenshotAlt: string;
    readonly loadRomRouterLink: string[];
    readonly clickTooltip: string;
    readonly authorsLinkTooltip: string;
    readonly romSiteLinkTooltip: string;
    readonly romSourceLinkTooltip: string;
    readonly romSourceIconName?: string;
}

function newFilterString(...values: Array<string | undefined>): string {
    return values
    // filter empty strings
        .map<string>(value => value || '')
        .filter(value => !!value)
        // convert everything to lower-case strings
        .map(value => value.toLocaleLowerCase())
        // join using '\n' as this character is not part of the filter-string
        //  => we won't filter across value boundaries
        .join('\n');
}
