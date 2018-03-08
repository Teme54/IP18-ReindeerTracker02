import { Injectable } from '@angular/core';
import { Http, Response } from "@angular/http";
import {HttpClientModule, HttpClient} from "@angular/common/http";
import { Observable } from "rxjs/Observable";
import "rxjs/add/operator/map";

@Injectable()
export class SensorDataService {

  private _ip: string = "ip adress server";


  constructor(private _http: HttpClient) { }

  getSensorData() {
    return this._http.get(this._ip).map((response => response));
}


}
