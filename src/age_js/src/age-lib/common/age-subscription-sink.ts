import {OnDestroy} from "@angular/core";
import {SubscriptionLike} from "rxjs";


export class AgeSubscriptionSink implements OnDestroy {

    private _subscriptions: SubscriptionLike[] = [];

    ngOnDestroy(): void {
        this.unsubscribeAll();
    }


    set newSubscription(subscription: SubscriptionLike) {
        this._subscriptions.push(subscription);
    }

    addSubscriptions(...subscriptions: SubscriptionLike[]): void {
        this._subscriptions = this._subscriptions.concat(...subscriptions);
    }

    unsubscribeAll(): void {
        this._subscriptions
            .filter(subscription => !subscription.closed)
            .forEach(subscription => subscription.unsubscribe());
        this._subscriptions = [];
    }
}
