import {ChangeDetectionStrategy, Component, Input} from '@angular/core';
import {AgeLoaderState} from './age-loader-state';


@Component({
    selector: 'age-loader-state-icon',
    template: `
        <ng-container [ngSwitch]="state">
            <i *ngSwitchCase="AgeLoaderState.WORKING" class="fa fa-cog fa-spin"></i>
            <i *ngSwitchCase="AgeLoaderState.SUCCESS" class="fa fa-check"></i>
            <i *ngSwitchCase="AgeLoaderState.ERROR" class="fa fa-times"></i>
        </ng-container>
    `,
    changeDetection: ChangeDetectionStrategy.OnPush
})
export class AgeLoaderStateIconComponent {

    readonly AgeLoaderState = AgeLoaderState;

    @Input()
    state: AgeLoaderState | undefined;
}
