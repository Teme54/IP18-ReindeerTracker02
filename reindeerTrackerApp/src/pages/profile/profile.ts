import { Component } from '@angular/core';
import { IonicPage, NavController, NavParams } from 'ionic-angular';
import {AuthProvider} from "../../providers/auth/auth";
import {SignUpPage} from "../sign-up/sign-up";
import { LinkProfelPage } from '../link-profel/link-profel';

/**
 * Generated class for the ProfilePage page.
 *
 * See https://ionicframework.com/docs/components/#navigation for more info on
 * Ionic pages and navigation.
 */

@IonicPage()
@Component({
  selector: 'page-profile',
  templateUrl: 'profile.html',
})
export class ProfilePage {
  currentUser = null;

  constructor(public navCtrl: NavController, public navParams: NavParams, private auth: AuthProvider) {
  }

  ionViewDidLoad() {
    this.auth.getUserDetails()
      .subscribe(res => {
        this.currentUser = res;
        console.log(this.currentUser);
      })
  }

  editProfile() {
    this.navCtrl.push(SignUpPage, this.currentUser);
  }

}
