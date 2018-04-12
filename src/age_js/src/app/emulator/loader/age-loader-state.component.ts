import {ChangeDetectionStrategy, Component, Input} from '@angular/core';
import {AgeLoaderState} from './age-loader-state';


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

            </ng-container>
        </div>
    `,
    changeDetection: ChangeDetectionStrategy.OnPush
})
export class AgeLoaderStateComponent {

    readonly AgeLoaderState = AgeLoaderState;

    @Input()
    state: AgeLoaderState | undefined;
}
