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
import {OnDestroy} from '@angular/core';
import {SubscriptionLike} from 'rxjs';


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
