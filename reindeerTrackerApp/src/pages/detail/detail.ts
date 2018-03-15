import { Component } from '@angular/core';
import { IonicPage, NavController, NavParams } from 'ionic-angular';
import { MapPage } from '../map/map';
import {DeviceDetails} from "../../classes/deviceDetails";
import {DeviceProvider} from "../../providers/device/device";
import {Device} from "../../classes/device";

/**
 * Generated class for the DetailPage page.
 *
 * See https://ionicframework.com/docs/components/#navigation for more info on
 * Ionic pages and navigation.
 */

@IonicPage()
@Component({
  selector: 'page-detail',
  templateUrl: 'detail.html',
})
export class DetailPage {
  device: Device = null;
  deviceKey = '123457';

  constructor(public navCtrl: NavController, public navParams: NavParams, private deviceProvider: DeviceProvider) {
  }

  ionViewDidLoad() {
    this.deviceKey = this.navParams.data;
    this.getDevice();
  }

  getDevice() {
    //TODO get deviceKey
    this.deviceProvider.getDevice(this.deviceKey)
      .subscribe((details: Device) => {
        this.device = details;
      })
  }

  goToMap(){
    this.navCtrl.push(MapPage);
  }

}
