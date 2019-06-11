//
// Copyright 2018 Christoph Sprenger
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

import {ChangeDetectionStrategy, Component, EventEmitter, Output} from "@angular/core";
import {Observable, of} from "rxjs";
import {map} from "rxjs/operators";


export type TAgeRomType = "demo" | "game";

export interface IAgeOnlineRom {
    readonly romType: TAgeRomType;
    readonly romTitle: string; // this is the primary key
    readonly romAuthors: string;
    readonly romAuthorsUrl?: string;
    readonly romSiteUrl?: string;
    readonly romUrl: string;
    readonly screenshotUrl?: string;
}

interface IAgeRomLibraryItem extends IAgeOnlineRom {
    readonly authorsLinkTooltip: string;
    readonly clickTooltip: string;
    readonly romSiteLinkTooltip: string;
    readonly screenshotTooltip: string;
}


@Component({
    selector: "age-rom-library",
    template: `
        <ng-container *ngIf="(onlineRoms$ | async) as libraryItems">
            <div *ngFor="let libraryItem of libraryItems; trackBy: trackByTitle"
                 class="library-item">

                <img *ngIf="libraryItem.screenshotUrl"
                     [alt]="libraryItem.screenshotTooltip"
                     (click)="romClicked.emit(libraryItem)"
                     [src]="libraryItem.screenshotUrl"
                     [title]="libraryItem.clickTooltip">

                <div class="details">
                    <div (click)="romClicked.emit(libraryItem)"
                         (keypress)="romClicked.emit(libraryItem)"
                         [title]="libraryItem.clickTooltip">{{libraryItem.romTitle}}</div>

                    <div>
                        by
                        <age-linkify [linkUrl]="libraryItem.romAuthorsUrl"
                                     [linkTooltip]="libraryItem.authorsLinkTooltip">
                            {{libraryItem.romAuthors}}
                        </age-linkify>
                    </div>

                    <age-linkify *ngIf="libraryItem.romSiteUrl as romSiteUrl"
                                 [allowIcon]="true"
                                 [linkUrl]="romSiteUrl"
                                 [linkTooltip]="libraryItem.romSiteLinkTooltip">
                        website
                    </age-linkify>
                </div>

            </div>
        </ng-container>
    `,
    styles: [`
        :host {
            display: flex;
            flex-direction: row;
            flex-wrap: wrap;
            align-items: center;
            justify-content: center;
        }

        .library-item {
            cursor: default;
            margin: 1em;
            width: 14em;
        }

        .library-item > img {
            cursor: pointer;
            object-fit: contain;
            max-width: 12em;
            max-height: 12em;
            margin: 1em 1em 0.25em;
        }

        .details {
            line-height: 1.6em;
            text-align: center;
            padding-left: 0.5em;
            padding-right: 0.5em;
        }

        .details > div {
            white-space: nowrap;
            text-overflow: ellipsis;
            overflow: hidden;
        }

        .details > :nth-child(1) {
            cursor: pointer;
            font-weight: bold;
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeOnlineRomLibraryComponent {

    @Output() readonly romClicked = new EventEmitter<IAgeOnlineRom>();

    readonly onlineRoms$: Observable<ReadonlyArray<IAgeRomLibraryItem>> = of(onlineRoms()).pipe(
        map(onlineRomList => onlineRomList.map(onlineRom => {
            return {
                ...onlineRom,
                authorsLinkTooltip: `${onlineRom.romAuthors} website`,
                clickTooltip: `run ${onlineRom.romTitle}`,
                romSiteLinkTooltip: `${onlineRom.romTitle} website`,
                screenshotTooltip: `${onlineRom.romTitle} screenshot`,
            };
        })),
    );

    trackByTitle(_index: number, libraryItem: IAgeRomLibraryItem): string {
        return libraryItem.romTitle;
    }
}


function onlineRoms(): IAgeOnlineRom[] {
    const roms: IAgeOnlineRom[] = [
        {
            romType: "demo",
            romTitle: "Back To Color",
            romAuthors: "AntonioND",
            romAuthorsUrl: "https://github.com/AntonioND",
            romSiteUrl: "https://github.com/AntonioND/back-to-color",
            romUrl: "https://github.com/AntonioND/back-to-color/archive/v1.1.zip",
            screenshotUrl: "https://github.com/AntonioND/back-to-color/raw/master/back_to_color_screenshots.png",
        },
        {
            romType: "demo",
            romTitle: "Cenotaph",
            romAuthors: "Dual Crew & Shining [DCS]",
            romAuthorsUrl: "https://www.pouet.net/groups.php?which=468",
            romSiteUrl: "https://www.pouet.net/prod.php?which=59139",
            romUrl: "https://gameboy.modermodemet.se/files/DCS-CTPH.ZIP",
            screenshotUrl: "https://content.pouet.net/files/screenshots/00059/00059139.jpg",
        },
        {
            romType: "demo",
            romTitle: "Cute Demo CGB",
            romAuthors: "Mills",
            romAuthorsUrl: "https://github.com/mills32",
            romSiteUrl: "https://github.com/mills32/CUTE_DEMO",
            romUrl: "https://github.com/mills32/CUTE_DEMO/raw/master/0_rom/CUTEDEMO.gbc",
            screenshotUrl: "https://github.com/mills32/CUTE_DEMO/raw/master/random_data/cutedemo.png",
        },
        {
            romType: "demo",
            romTitle: "Demotronic",
            romAuthors: "1.000.000 boys [1MB]",
            romAuthorsUrl: "https://www.pouet.net/groups.php?which=1237",
            romSiteUrl: "https://www.pouet.net/prod.php?which=7175",
            romUrl: "https://gameboy.modermodemet.se/files/MB-DTRNC.ZIP",
            screenshotUrl: "https://www.pouet.net/content/files/screenshots/00007/00007175.gif",
        },
        {
            romType: "demo",
            romTitle: "It Came from Planet Zilog",
            romAuthors: "phantasy",
            romAuthorsUrl: "https://www.pouet.net/groups.php?which=754",
            romSiteUrl: "https://www.pouet.net/prod.php?which=65345",
            romUrl: "https://gameboy.modermodemet.se/files/PHT-PZ.ZIP",
            screenshotUrl: "https://www.pouet.net/content/files/screenshots/00065/00065345.gif",
        },
        {
            romType: "demo",
            romTitle: "Mental Respirator",
            romAuthors: "phantasy",
            romAuthorsUrl: "https://www.pouet.net/groups.php?which=754",
            romSiteUrl: "https://www.pouet.net/prod.php?which=16402",
            romUrl: "http://gameboy.modermodemet.se/files/PHT-MR.ZIP",
            screenshotUrl: "https://www.pouet.net/content/files/screenshots/00016/00016402.gif",
        },
        {
            romType: "demo",
            romTitle: "Oh!",
            romAuthors: "Snorpung",
            romAuthorsUrl: "https://www.pouet.net/groups.php?which=10735",
            romSiteUrl: "https://www.pouet.net/prod.php?which=54175",
            romUrl: "https://www.nordloef.com/gbdemos/oh.zip",
            screenshotUrl: "https://www.pouet.net/content/files/screenshots/00054/00054175.jpg",
        },
        {
            romType: "demo",
            romTitle: "Pickpocket",
            romAuthors: "inka",
            romAuthorsUrl: "https://www.pouet.net/groups.php?which=1328",
            romSiteUrl: "https://www.pouet.net/prod.php?which=5681",
            romUrl: "https://www.izik.se/files/demos/inka-PP.zip",
            screenshotUrl: "https://www.pouet.net/content/files/screenshots/00005/00005681.gif",
        },
    ];

    return roms.sort(
        (aRom: IAgeOnlineRom, bRom: IAgeOnlineRom) => aRom.romTitle.localeCompare(bRom.romTitle),
    );
}
