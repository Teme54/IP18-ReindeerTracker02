import { Injectable } from '@angular/core';
import {AngularFireAuth} from "angularfire2/auth";
import {Userdata} from "./userdata";
import {HttpClient} from "@angular/common/http";
import {AppSettings} from "./AppSettings";
import {Observable} from "rxjs/Observable";
import {User} from "firebase/app";

@Injectable()
export class AuthService {
  url = AppSettings.API_ENDPOINT;
  currentUser = null;

  constructor(private af: AngularFireAuth, private http: HttpClient) { }

  getCurrentUID(): string {
    return this.af.auth.currentUser.uid;
  }

  getAuthToken(): Promise<any> {
    return this.af.auth.currentUser.getIdToken();
  }

  isAuthenticated(): boolean {
    if (this.af.auth.currentUser) {
      return true;
    } else {
      return false;
    }
  }

  loginWithEmailPassword(email: string, password: string): Promise<any> {
    return this.af.auth.signInWithEmailAndPassword(email, password)
  }

  signupWithEmailPassword(email: string, password: string, data: Userdata): Observable<any> {
    const observable = new Observable(observer => {
      this.af.auth.createUserWithEmailAndPassword(email, password)
        .then(res => {
          data.uid = res.uid;
          console.log(res.uid);
          this.http.post(this.url + '/users', data)
            .subscribe(ress => {
              observer.next(ress);
              console.log(ress);
            }, err => {
              if (err.status >= 200 && err.status < 300) {
                observer.next();
              } else {
                observer.error(err);
              }
              console.log(err);
            })
        })
        .catch(err => {
          observer.error(err);
        })
    });
    return observable;
  }

  resetPassword(email: string): Promise<any> {
    return this.af.auth.sendPasswordResetEmail(email);
  }

  signOut(): Promise<any> {
    return this.af.auth.signOut();
  }

  setCurrentUser(): Promise<any> {
    return new Promise((resolve, reject) => {
      if (this.isAuthenticated()) {
        this.http.get(this.url + '/users/' + this.getCurrentUID())
          .subscribe(res => {
            this.currentUser = res;
            resolve(res);
            console.log(res);
          })
      } else {
        reject('Not logged in');
      }
    });
  }

  getCurrentUser() {
    return this.currentUser;
  }

  isAdmin() {
    if (this.currentUser) {
      return this.currentUser.admin;
    } else {
      return false
    }
  }

}
