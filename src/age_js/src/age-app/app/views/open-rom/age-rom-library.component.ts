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

import {ChangeDetectionStrategy, Component, Input, OnInit} from '@angular/core';
import {AgeIconsService} from 'age-lib';
import {BehaviorSubject, combineLatest, Observable, of} from 'rxjs';
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
                    <img *ngIf="rom.romScreenshotUrl"
                         mat-card-image
                         [alt]="rom.screenshotAlt"
                         [src]="rom.romScreenshotUrl">
                </a>

                <mat-card-title>{{rom.romTitle}}</mat-card-title>

                <mat-card-subtitle>
                    {{rom.romType}}<br>
                    by
                    <a class="age-ui-clickable"
                       [age-href]="rom.romAuthorsUrl"
                       [title]="rom.authorsLinkTooltip">
                        {{rom.romAuthors}}
                    </a>
                </mat-card-subtitle>

                <mat-card-actions>

                    <a mat-button
                       [routerLink]="rom.loadRomRouterLink"
                       [title]="rom.clickTooltip">
                        <mat-icon [svgIcon]="icons.faPlay"></mat-icon>
                    </a>

                    <a *ngIf="rom.romSiteUrl as romSiteUrl"
                       mat-button
                       [age-href]="romSiteUrl"
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
                readonly navigationService: AgeNavigationService) {
    }

    ngOnInit(): void {
        const roms$: Observable<IAgeRomLibraryItem[]> = of(onlineRoms()).pipe(
            map(roms => roms.map(rom => {
                const filterString = newFilterString(
                    rom.romType,
                    rom.romTitle,
                    rom.romAuthors,
                );
                return {
                    ...rom,
                    filterString,
                    screenshotAlt: `${rom.romTitle} screenshot`,
                    loadRomRouterLink: this.navigationService.romUrlRouterLink(rom.romUrl),
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


type TAgeRomType = 'demo' | 'game';

interface IAgeOnlineRom {
    readonly romType: TAgeRomType;
    readonly romTitle: string; // this is the primary key
    readonly romAuthors: string;
    readonly romUrl: string;
    readonly romScreenshotUrl?: string;
    readonly romAuthorsUrl?: string;
    readonly romSiteUrl?: string;
    readonly romSourceUrl?: string;
}

interface IAgeRomLibraryItem extends IAgeOnlineRom {
    readonly filterString: string;
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

function onlineRoms(): IAgeOnlineRom[] {
    const roms: IAgeOnlineRom[] = [
        {
            romType: 'demo',
            romTitle: 'Back To Color',
            romAuthors: 'SkyLyrac',
            romUrl: 'https://github.com/AntonioND/back-to-color/archive/v1.1.zip',
            romScreenshotUrl: 'https://www.pouet.net/content/files/screenshots/00063/00063691.png',
            romAuthorsUrl: 'https://www.pouet.net/user.php?who=98359',
            romSiteUrl: 'https://www.pouet.net/prod.php?which=63691',
            romSourceUrl: 'https://github.com/AntonioND/back-to-color',
        },
        {
            romType: 'demo',
            romTitle: 'Cenotaph',
            romAuthors: 'Dual Crew & Shining [DCS]',
            romUrl: 'https://gameboy.modermodemet.se/files/DCS-CTPH.ZIP',
            romScreenshotUrl: 'https://content.pouet.net/files/screenshots/00059/00059139.jpg',
            romAuthorsUrl: 'https://www.pouet.net/groups.php?which=468',
            romSiteUrl: 'https://www.pouet.net/prod.php?which=59139',
        },
        {
            romType: 'demo',
            romTitle: 'Cute Demo CGB',
            romAuthors: 'Mills',
            romUrl: 'https://github.com/mills32/CUTE_DEMO/raw/master/0_rom/CUTEDEMO.gbc',
            romScreenshotUrl: 'https://www.pouet.net/content/files/screenshots/00073/00073290.png',
            romAuthorsUrl: 'https://www.pouet.net/user.php?who=98229',
            romSiteUrl: 'https://www.pouet.net/prod.php?which=73290',
            romSourceUrl: 'https://github.com/mills32/CUTE_DEMO',
        },
        {
            romType: 'demo',
            romTitle: 'Demotronic',
            romAuthors: '1.000.000 boys [1MB]',
            romUrl: 'https://gameboy.modermodemet.se/files/MB-DTRNC.ZIP',
            romScreenshotUrl: 'https://www.pouet.net/content/files/screenshots/00007/00007175.gif',
            romAuthorsUrl: 'https://www.pouet.net/groups.php?which=1237',
            romSiteUrl: 'https://www.pouet.net/prod.php?which=7175',
        },
        {
            romType: 'demo',
            romTitle: 'It Came from Planet Zilog',
            romAuthors: 'phantasy',
            romUrl: 'https://gameboy.modermodemet.se/files/PHT-PZ.ZIP',
            romScreenshotUrl: 'https://www.pouet.net/content/files/screenshots/00065/00065345.gif',
            romAuthorsUrl: 'https://www.pouet.net/groups.php?which=754',
            romSiteUrl: 'https://www.pouet.net/prod.php?which=65345',
        },
        {
            romType: 'demo',
            romTitle: 'Mental Respirator',
            romAuthors: 'phantasy',
            romUrl: 'http://gameboy.modermodemet.se/files/PHT-MR.ZIP',
            romScreenshotUrl: 'https://www.pouet.net/content/files/screenshots/00016/00016402.gif',
            romAuthorsUrl: 'https://www.pouet.net/groups.php?which=754',
            romSiteUrl: 'https://www.pouet.net/prod.php?which=16402',
        },
        {
            romType: 'demo',
            romTitle: 'Oh!',
            romAuthors: 'Snorpung',
            romUrl: 'https://www.nordloef.com/gbdemos/oh.zip',
            romScreenshotUrl: 'https://www.pouet.net/content/files/screenshots/00054/00054175.jpg',
            romAuthorsUrl: 'https://www.pouet.net/groups.php?which=10735',
            romSiteUrl: 'https://www.pouet.net/prod.php?which=54175',
        },
        {
            romType: 'demo',
            romTitle: 'Pickpocket',
            romAuthors: 'inka',
            romUrl: 'https://www.izik.se/files/demos/inka-PP.zip',
            romScreenshotUrl: 'https://www.pouet.net/content/files/screenshots/00005/00005681.gif',
            romAuthorsUrl: 'https://www.pouet.net/groups.php?which=1328',
            romSiteUrl: 'https://www.pouet.net/prod.php?which=5681',
        },
        {
            romType: 'demo',
            romTitle: 'Roboto',
            romAuthors: 'naavis',
            romUrl: 'https://naavis.untergrund.net/skrolliparty2017/naavis_roboto_skrolli-party.gb',
            romScreenshotUrl: 'https://www.pouet.net/content/files/screenshots/00069/00069944.jpg',
            romAuthorsUrl: 'https://www.pouet.net/user.php?who=97913',
            romSiteUrl: 'https://www.pouet.net/prod.php?which=69944',
            romSourceUrl: 'https://github.com/naavis/roboto-demo',
        },
    ];

    return roms.sort(
        (aRom: IAgeOnlineRom, bRom: IAgeOnlineRom) => aRom.romTitle.localeCompare(bRom.romTitle),
    );
}
