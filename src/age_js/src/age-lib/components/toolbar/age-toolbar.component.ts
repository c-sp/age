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

import {ChangeDetectionStrategy, Component, Input} from "@angular/core";


@Component({
    selector: "age-toolbar",
    template: `
        <mat-toolbar class="just-for-the-background"
                     [ngStyle]="bgStyle"></mat-toolbar>

        <mat-toolbar class="actual-toolbar">
            <ng-content></ng-content>
        </mat-toolbar>
    `,
    styles: [`
        .just-for-the-background {
            opacity: 0.65;
            position: absolute;
            left: 0;
            top: 0;
        }

        .actual-toolbar {
            background: none;
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeToolbarComponent {

    private _bgStyle = {
        opacity: 0.65,
    };

    get bgStyle(): object {
        return this._bgStyle;
    }

    @Input() set backgroundOpacity(backgroundOpacity: number) {
        this._bgStyle.opacity = backgroundOpacity;
    }
}
