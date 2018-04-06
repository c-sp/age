import {ChangeDetectionStrategy, Component, Input} from '@angular/core';
import {TaskState} from './task-state';


@Component({
    selector: 'age-task-state',
    template: `
        <ng-container [ngSwitch]="state">
            <i *ngSwitchCase="TaskState.WORKING" class="fa fa-cog fa-spin"></i>
            <i *ngSwitchCase="TaskState.SUCCESS" class="fa fa-check"></i>
            <i *ngSwitchCase="TaskState.ERROR" class="fa fa-times"></i>
        </ng-container>
    `,
    changeDetection: ChangeDetectionStrategy.OnPush
})
export class AgeTaskStateComponent {

    readonly TaskState = TaskState;

    @Input()
    state: TaskState | undefined;
}
