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

import {ChangeDetectionStrategy, Component, Input} from '@angular/core';


export enum AgeLoaderState {
    WORKING,
    SUCCESS,
    ERROR
}


@Component({
    selector: 'age-loader-state',
    template: `
        <div>
            <ng-container [ngSwitch]="state">
                <i *ngSwitchCase="AgeLoaderState.WORKING" class="fa fa-cog fa-spin"></i>
                <i *ngSwitchCase="AgeLoaderState.SUCCESS" class="fa fa-check"></i>
                <i *ngSwitchCase="AgeLoaderState.ERROR" class="fa fa-times"></i>
            </ng-container>

            <ng-container [ngSwitch]="state">

                <ng-container *ngSwitchCase="AgeLoaderState.WORKING">
                    <ng-content select="[ageLoaderWorking]"></ng-content>
                </ng-container>

                <ng-container *ngSwitchCase="AgeLoaderState.SUCCESS">
                    <ng-content select="[ageLoaderSuccess]"></ng-content>
                </ng-container>

                <ng-container *ngSwitchCase="AgeLoaderState.ERROR">
                    <ng-content select="[ageLoaderError]"></ng-content>
                </ng-container>

                <span *ngSwitchDefault>
                    <ng-content select="[ageLoaderNoState]"></ng-content>
                </span>

            </ng-container>
        </div>
    `,
    styles: [`
        i {
            margin-right: .5em;
        }

        span {
            color: #505050;
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush
})
export class AgeLoaderStateComponent {

    readonly AgeLoaderState = AgeLoaderState;

    @Input() state?: AgeLoaderState;
}
