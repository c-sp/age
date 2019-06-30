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

import {ChangeDetectionStrategy, Component, HostBinding, Input} from '@angular/core';
import {AgeIconsService} from 'age-lib';
import {Observable, of} from 'rxjs';
import {map} from 'rxjs/operators';
import {AgeNavigationService} from '../../common';


// TODO check IAgeRomLibraryItem attribute usage

@Component({
    selector: 'age-rom-library',
    template: `
        <ng-container *ngIf="(onlineRoms$ | async) as libraryItems">

            <mat-card *ngFor="let libraryItem of libraryItems; trackBy: trackByTitle">
                <mat-card-header>
                    <mat-card-title>{{libraryItem.romTitle}}</mat-card-title>
                </mat-card-header>

                <img *ngIf="libraryItem.romScreenshotUrl"
                     mat-card-image
                     [alt]="libraryItem.screenshotAlt"
                     [src]="libraryItem.romScreenshotUrl">

                <mat-card-content>
                    by
                    <a class="age-ui-clickable"
                       [age-href]="libraryItem.romAuthorsUrl"
                       [title]="libraryItem.authorsLinkTooltip">
                        {{libraryItem.romAuthors}}
                    </a>
                </mat-card-content>

                <mat-card-actions>

                    <a mat-button [title]="libraryItem.clickTooltip">
                        <mat-icon [svgIcon]="icons.faPlay"></mat-icon>
                    </a>

                    <a *ngIf="libraryItem.romSiteUrl as romSiteUrl"
                       mat-button
                       [age-href]="romSiteUrl"
                       [title]="libraryItem.romSiteLinkTooltip">
                        <mat-icon [svgIcon]="icons.faHome"></mat-icon>
                    </a>

                    <a *ngIf="libraryItem.romSourceUrl as romSourceUrl"
                       mat-button
                       [age-href]="romSourceUrl"
                       [title]="libraryItem.romSourceLinkTooltip">
                        <mat-icon [svgIcon]="libraryItem.romSourceIconName"></mat-icon>
                    </a>

                </mat-card-actions>

            </mat-card>

        </ng-container>
    `,
    styles: [`
        :host {
            display: flex;
            flex-direction: row;
            flex-wrap: wrap;
            align-items: center;
        }

        mat-card {
            /* width required for 3 mat-card-actions */
            width: 240px;
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeRomLibraryComponent {

    readonly onlineRoms$: Observable<ReadonlyArray<IAgeRomLibraryItem>> = of(onlineRoms()).pipe(
        map<IAgeOnlineRom[], IAgeRomLibraryItem[]>(onlineRomList => onlineRomList.map(onlineRom => {
            return {
                ...onlineRom,
                screenshotAlt: `${onlineRom.romTitle} screenshot`,
                loadRomRouterLink: this.navigationService.romUrlRouterLink(onlineRom.romUrl),
                clickTooltip: `run ${onlineRom.romTitle}`,
                authorsLinkTooltip: `${onlineRom.romAuthors} on pouet.net`,
                romSiteLinkTooltip: `${onlineRom.romTitle} on pouet.net`,
                romSourceLinkTooltip: `${onlineRom.romTitle} source code`,
                romSourceIconName: this._sourceLinkIconName(onlineRom.romSourceUrl),
            };
        })),
    );

    @Input() @HostBinding('style.justifyContent') justifyContent: 'normal' | 'center' = 'normal';

    constructor(readonly icons: AgeIconsService,
                readonly navigationService: AgeNavigationService) {
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
    readonly screenshotAlt: string;
    readonly loadRomRouterLink: string[];
    readonly clickTooltip: string;
    readonly authorsLinkTooltip: string;
    readonly romSiteLinkTooltip: string;
    readonly romSourceLinkTooltip: string;
    readonly romSourceIconName?: string;
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
