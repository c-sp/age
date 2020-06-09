import {Injectable, OnDestroy} from '@angular/core';
import {TAgeRomFile} from 'age-lib';
import {BehaviorSubject, Observable} from 'rxjs';


@Injectable()
export class AgeRomFileService implements OnDestroy {

    // to counter any race conditions during component initialization the
    // current rom file is cached by the subject
    private readonly _openRomFileSubject = new BehaviorSubject<TAgeRomFile | undefined>(undefined);
    private readonly _openRomFile$ = this._openRomFileSubject.asObservable();

    ngOnDestroy(): void {
        this._openRomFileSubject.complete();
    }

    get openRomFile$(): Observable<TAgeRomFile | undefined> {
        return this._openRomFile$;
    }

    openRomFile(romFile: TAgeRomFile | undefined): void {
        this._openRomFileSubject.next(romFile);
    }
}
