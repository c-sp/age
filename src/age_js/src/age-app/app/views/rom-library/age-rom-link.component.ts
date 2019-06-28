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

import {ChangeDetectionStrategy, Component, Input} from '@angular/core';
import {AgeIconsService} from 'age-lib';


@Component({
    selector: 'age-rom-link',
    template: `
        <a *ngIf="linkUrl; else contents"
           class="age-ui-clickable"
           [age-href]="linkUrl"
           [title]="linkTooltip || ''">

            <ng-container *ngTemplateOutlet="contents"></ng-container>
        </a>

        <ng-template #contents>
            <mat-icon *ngIf="iconName" [svgIcon]="iconName"></mat-icon>
            <ng-content *ngIf="!iconName"></ng-content>
        </ng-template>
    `,
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeRomLinkComponent {

    @Input() linkTooltip?: string;

    private _autoIcon = false;
    private _iconName?: string;
    private _linkUrl?: string;

    constructor(private readonly _icons: AgeIconsService) {
    }

    @Input() set autoIcon(autoIcon: boolean) {
        this._autoIcon = autoIcon;
        this._updateIcon();
    }

    get iconName(): string | undefined {
        return this._iconName;
    }

    get linkUrl(): string | undefined {
        return this._linkUrl || '';
    }

    @Input() set linkUrl(linkUrl: string | undefined) {
        this._linkUrl = linkUrl;
        this._updateIcon();
    }


    private _updateIcon() {
        const linkHostname = (this._autoIcon && this._linkUrl) ? new URL(this._linkUrl).hostname : '';
        switch (linkHostname) {

            case 'github.com':
                this._iconName = this._icons.faGithub;
                break;

            case 'gitlab.com':
                this._iconName = this._icons.faGitlab;
                break;

            default:
                this._iconName = undefined;
                break;
        }
    }
}
