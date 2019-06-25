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

import {ChangeDetectionStrategy, Component, EventEmitter, Input, Output} from "@angular/core";
import {IconDefinition} from "@fortawesome/fontawesome-svg-core";


@Component({
    selector: "age-toolbar-action",
    template: `
        <button mat-button
                [disabled]="disabled"
                (click)="click($event)">

            <fa-icon *ngIf="icon" [icon]="icon"></fa-icon>
            <mat-icon *ngIf="matIconName" [svgIcon]="matIconName"></mat-icon>

        </button>
    `,
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeToolbarActionComponent {

    @Input() disabled = false;
    @Input() icon?: IconDefinition;
    @Input() matIconName?: string;

    @Output() readonly clicked = new EventEmitter();

    click(event: Event): void {
        this.clicked.emit();
        // don't propagate any further as this might cause the toolbar to disappear
        event.stopPropagation();
    }
}
