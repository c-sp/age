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
    selector: "age-toolbar-background",
    template: `
        <div class="mat-toolbar background" [ngStyle]="bgStyle"></div>
        <ng-content></ng-content>
    `,
    styles: [`
        :host {
            /*
             calculate ".background" element dimensions based on this element's dimensions
             (which in turn is based on it's content excluding the absolute positioned elements)
              */
            display: block;
            /* align ".background" element relative to this one */
            position: relative;
        }

        .background {
            position: absolute;
            left: 0;
            top: 0;
            width: 100%;
            height: 100%;
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeToolbarBackgroundComponent {

    private _bgStyle = {
        opacity: 0.7,
    };

    get bgStyle(): object {
        return this._bgStyle;
    }

    @Input() set backgroundOpacity(backgroundOpacity: number) {
        this._bgStyle.opacity = backgroundOpacity;
    }
}
