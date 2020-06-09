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

import {Injectable} from '@angular/core';
import {MatIconRegistry} from '@angular/material/icon';
import {DomSanitizer} from '@angular/platform-browser';
import {IconDefinition} from '@fortawesome/fontawesome-svg-core';
import {faGithub} from '@fortawesome/free-brands-svg-icons/faGithub';
import {faGitlab} from '@fortawesome/free-brands-svg-icons/faGitlab';
import {faAngleLeft} from '@fortawesome/free-solid-svg-icons/faAngleLeft';
import {faCheck} from '@fortawesome/free-solid-svg-icons/faCheck';
import {faCog} from '@fortawesome/free-solid-svg-icons/faCog';
import {faDesktop} from '@fortawesome/free-solid-svg-icons/faDesktop';
import {faEllipsisV} from '@fortawesome/free-solid-svg-icons/faEllipsisV';
import {faExclamationCircle} from '@fortawesome/free-solid-svg-icons/faExclamationCircle';
import {faFileCode} from '@fortawesome/free-solid-svg-icons/faFileCode';
import {faGlobeAmericas} from '@fortawesome/free-solid-svg-icons/faGlobeAmericas';
import {faHome} from '@fortawesome/free-solid-svg-icons/faHome';
import {faInfoCircle} from '@fortawesome/free-solid-svg-icons/faInfoCircle';
import {faMobileAlt} from '@fortawesome/free-solid-svg-icons/faMobileAlt';
import {faPause} from '@fortawesome/free-solid-svg-icons/faPause';
import {faPlay} from '@fortawesome/free-solid-svg-icons/faPlay';
import {faSearch} from '@fortawesome/free-solid-svg-icons/faSearch';
import {faTimes} from '@fortawesome/free-solid-svg-icons/faTimes';
import {faVolumeMute} from '@fortawesome/free-solid-svg-icons/faVolumeMute';
import {faVolumeUp} from '@fortawesome/free-solid-svg-icons/faVolumeUp';


/**
 * AGE makes use of different icons
 * (at the time of writing: custom icon assets and {@link https://fontawesome.com|Font Awesome} icons).
 *
 * For unified icon layout we only use `<mat-icon>`.
 * Other elements like `<fa-icon>` are not used.
 *
 * Purpose of the `AgeIconsService` is to prepare and register all icons with the {@link MatIconRegistry}
 * so that they can easily be used with `<mat-icon [svgIcon]='icon_name'></mat-icon>`.
 */
@Injectable({
    providedIn: 'root',
})
export class AgeIconsService {

    readonly ageCartridge = this._registerIconUrl('age-cartridge', 'assets/icons/cartridge.svg');

    readonly faAngleLeft = this._registerFaIcon(faAngleLeft);
    readonly faCog = this._registerFaIcon(faCog);
    readonly faCheck = this._registerFaIcon(faCheck);
    readonly faDesktop = this._registerFaIcon(faDesktop);
    readonly faEllipsisV = this._registerFaIcon(faEllipsisV);
    readonly faExclamationCircle = this._registerFaIcon(faExclamationCircle);
    readonly faFileCode = this._registerFaIcon(faFileCode);
    readonly faGithub = this._registerFaIcon(faGithub);
    readonly faGitlab = this._registerFaIcon(faGitlab);
    readonly faGlobeAmericas = this._registerFaIcon(faGlobeAmericas);
    readonly faHome = this._registerFaIcon(faHome);
    readonly faInfoCircle = this._registerFaIcon(faInfoCircle);
    readonly faMobileAlt = this._registerFaIcon(faMobileAlt);
    readonly faPause = this._registerFaIcon(faPause);
    readonly faPlay = this._registerFaIcon(faPlay);
    readonly faSearch = this._registerFaIcon(faSearch);
    readonly faTimes = this._registerFaIcon(faTimes);
    readonly faVolumeMute = this._registerFaIcon(faVolumeMute);
    readonly faVolumeUp = this._registerFaIcon(faVolumeUp);


    constructor(private readonly _matIconRegistry: MatIconRegistry,
                private readonly _domSanitizer: DomSanitizer) {
    }

    private _registerIconUrl(iconName: string, iconUrl: string): string {
        const safeUrl = this._domSanitizer.bypassSecurityTrustResourceUrl(iconUrl);
        this._matIconRegistry.addSvgIcon(iconName, safeUrl);
        return iconName;
    }

    private _registerFaIcon(iconDefinition: IconDefinition): string {
        const svgPath = iconDefinition.icon[4];
        if (Array.isArray(svgPath) || !svgPath.match(/[0-9.\-\sa-zA-Z]+/)) {
            throw new Error(`FA icon "${iconDefinition.iconName}": invalid SVG path`);
        }
        const width = iconDefinition.icon[0];
        const height = iconDefinition.icon[1];
        const viewBox = `0 0 ${width} ${height}`;

        const html = `<svg xmlns="http://www.w3.org/2000/svg" viewBox="${viewBox}"><path d="${svgPath}"/></svg>`;
        const safeHtml = this._domSanitizer.bypassSecurityTrustHtml(html);

        const iconName = `${iconDefinition.prefix}-${iconDefinition.iconName}`;
        this._matIconRegistry.addSvgIconLiteral(iconName, safeHtml);

        return iconName;
    }
}
