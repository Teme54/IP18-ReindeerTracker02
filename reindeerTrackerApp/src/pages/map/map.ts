import { Component, ViewChild, ElementRef } from '@angular/core';
import {IonicPage, NavController, NavParams, Platform} from 'ionic-angular';
import { LaunchNavigator} from '@ionic-native/launch-navigator'
import {DateformatProvider} from "../../providers/dateformat/dateformat";

/**
 * Generated class for the MapPage page.
 *
 * See https://ionicframework.com/docs/components/#navigation for more info on
 * Ionic pages and navigation.
 */

declare var google: any;

@IonicPage()
@Component({
  selector: 'page-map',
  templateUrl: 'map.html',
})
export class MapPage {

  @ViewChild('map') mapRef: ElementRef;

  map: any;
  lat: number;
  long: number;
  name: string;
  gender: string;
  lastSignal: Date;
  battery: string;
  DA: string;


  constructor(public navCtrl: NavController, public navParams: NavParams, private launchNavigator: LaunchNavigator, private platform: Platform, private dateFormatProvider: DateformatProvider) {

  }

  loadMaps() {
    this.lat = this.navParams.get('lat');
    this.long = this.navParams.get('long');
    this.name = this.navParams.get('name');
    this.gender = this.navParams.get('gender');
    this.lastSignal = new Date(this.navParams.get('lastSignal'));
    this.battery = this.navParams.get('battery');
    this.DA = this.navParams.get('DA');


    this.showMap();

    this.addRadius(new google.maps.LatLng(this.lat, this.long), this.map);
    this.addMarker(new google.maps.LatLng(this.lat, this.long), this.map);

  }

  ionViewDidLoad(){
    this.loadMaps();
  }

  showMap(){
    //location - lat long
    const location = new google.maps.LatLng(this.lat, this.long);

    //map options
    const options = {
      center: location,
      zoom : 13,
      streetViewControl: false,
      mapTypeId: 'terrain'
    }

    this.map = new google.maps.Map(this.mapRef.nativeElement, options);
  }

  addRadius(position, map){
    return new google.maps.Circle({
      strokeColor: '#FF0000',
      strokeOpacity: 0.8,
      strokeWeight: 2,
      fillColor: '#FF0000',
      fillOpacity: 0.35,
      map: map,
      center: position,
      radius: 1000
    });
  }

  addMarker(position, map){
    var iconBase = 'https://maps.google.com/mapfiles/kml/shapes/'

    var contentString =
      '<div id="content">'+
        '<div id="siteNotice">'+
        '</div>'+
        '<h1 id="firstHeading" class="firstHeading">' + this.name + '</h1>'+
        '<div id="bodyContent">'+
          '<p><b>gender: </b>' + this.gender +
          '<p><b>Last signal: </b>'+ this.lastSignal.toDateString() +
          '<p><b>battery: </b>'+ this.battery + "%" +
          '<p><b>death/alive: </b>'+ this.DA +
        '</div>'+
      '</div>';

    var infowindow = new google.maps.InfoWindow({
      content: contentString
    });

   var marker =  new google.maps.Marker({
      position,
      map,
      icon: 'assets/imgs/reindeerMarker.png'
    })

    marker.addListener('click', function() {
      infowindow.open(map, marker);
    });

    return marker;
  }

  navMe(){
    const position = this.lat + ',' + this.long;
    this.launchNavigator.navigate(position);
  }

}
