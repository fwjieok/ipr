'use strict'

var util = require('util');
var net  = require("net");

var argv = process.argv;

if (argv.length < 7) {
    console.log("Usage: node " + argv[1] + " <ip> <port> <event_number> <connections> <event_time_delay>");
    return;
}

var ip                       = argv[2];
var port                     = argv[3];
var total_event_number       = argv[4];
var max_connections          = argv[5] || 100;
var event_time_delay         = argv[6] || 60;;
var event_number_per_client  = Math.round(total_event_number / max_connections);

var heart_beat_time           = 15;
var connections               = [];     
var connection_number_ok      = 0;
var connection_number_timeout = 0;
var connection_number_error   = 0;
var complete_number           = 0;

process.on('SIGINT', function () {
    console.log("");
    print_connect_status();
    print_connect_time();
    print_send_event_number();
    process.exit(-1);
});

function print_send_event_number() {
    if (complete_number >= connection_number_ok) {
        connections = null;
        console.log("Total Event Send: " + (connection_number_ok * event_number_per_client));
    }
}

function print_connect_time() {
    var connect_time_0_3  = 0;
    var connect_time_4_10 = 0;
    var connect_time_10   = 0;

    for (var i = 0; i < connections.length; i ++ ) {
        if (connections[i].connect_ok) {
            if ((connections[i].connect_time <= 3) && (connections[i].connect_time >= 0)) {
                connect_time_0_3 ++;
            } else if ((connections[i].connect_time > 3) && (connections[i].connect_time <= 10)) {
                connect_time_4_10 ++;
            } else {
                connect_time_10 ++;
            }
        }
    }
    console.log("connect time within 0-3  seconds: "   + connect_time_0_3);
    console.log("connect time within 4-10 seconds: "    + connect_time_4_10);
    console.log("connect time in 10 seconds or moreS: " + connect_time_10);
}

function print_connect_status() {
    var connect_number = connection_number_ok + connection_number_timeout + connection_number_error;
    if (connect_number >= max_connections) {
        console.log("Total connections number: " + connections.length);
        console.log("connect success   number: " + connection_number_ok);
        console.log("connect timeout   number: " + connection_number_timeout);
        console.log("connect error     number: " + connection_number_error);

        print_connect_time();
    }
}


function Client(ip, port, name, event_number) {
    this.ip   = ip;
    this.port = port;
    this.name = name;
    this.event_number = event_number;

    this.connect_ok         = false;
    this.connect_time       = 0;
    this.connect_time_start = 0;
    this.connect_time_end   = 0;

    this.rollseq = 0;
}

Client.prototype.go = function () {
    this.connect_time_start = (new Date()).getTime();
    
    this.socket = net.connect(this.port, this.ip, this.on_connected.bind(this));
    this.socket.on('timeout', this.on_timeout.bind(this));
    this.socket.on("data",    this.on_data.bind(this));
    this.socket.on("error",   this.on_error.bind(this));
    this.socket.on("end",     this.on_end.bind(this));
    this.socket.on("close",   this.on_close.bind(this));
}

Client.prototype.on_connected = function () {
    this.connect_ok = true;
    connection_number_ok ++;

    print_connect_status();
    
    this.connect_time_end = (new Date()).getTime();
    this.connect_time     = (this.connect_time_end - this.connect_time_start)/1000;

    if (!this.timer_heart_beat) {
        this.timer_heart_beat = setInterval(this.send_heart_beat.bind(this), (Math.round(5 * Math.random()) + heart_beat_time) * 1000);
    }

    if (!this.timer_event) {
        this.timer_event = setInterval(this.send_event.bind(this), event_time_delay * 1000);
    }
}

Client.prototype.on_timeout = function(data) {
    this.connect_ok = false;
    connection_number_timeout ++;

    this.socket = null;
    print_connect_status();
}

Client.prototype.on_error = function(error) {
    console.log(error);
    connection_number_error ++;
    print_connect_status();
    
    if (this.timer_heart_beat) {
        clearInterval(this.timer_heart_beat);
    }
    if (this.timer_event) {
        clearInterval(this.timer_event);
    }
    this.socket.destroy();
}

Client.prototype.on_end = function() {

    if (this.timer_heart_beat) {
        clearInterval(this.timer_heart_beat);
    }
    if (this.timer_event) {
        clearInterval(this.timer_event);
    }

    this.connect_ok = false;
    this.socket = null;
}

Client.prototype.on_close = function() {
    
    if (this.timer_heart_beat) {
        clearInterval(this.timer_heart_beat);
    }
    if (this.timer_event) {
        clearInterval(this.timer_event);
    }

    this.connect_ok = false;
    this.socket = null;
}

Client.prototype.on_data = function(data) {
    var arr = data.toString().split(',');
    if (arr[0].trim() == "ack") {
        this.rollseq = ++this.rollseq % 10000;
    }
}

Client.prototype.send = function(package_type, rollseq, package_data) {

    var data = '\r\n' + package_type + ',' + rollseq + ',' + package_data + '\r\n';
    this.socket.write(data);
}

Client.prototype.send_heart_beat = function() {
    if (this.rollseq == 0) {
        //握手包
        this.send('d', this.rollseq, 'hi,id:'  + " 0000" + this.name);
    } else {
        //心跳包
        var time = (new Date()).Format("yyyy-MM-dd hh:mm:ss");
        this.send('d', this.rollseq, 'hi,id:0000' + this.name + ','
                  + 'status:bosch,sn:TEST-53R-P5-NG9,time:'+ time +',level:0,ver:14');
    }
}

Client.prototype.send_event = function() {
    if (this.event_number <= 0) {
        complete_number ++;
        if (this.timer_event) { clearInterval(this.timer_event); }
        if (this.timer_heart_beat) { clearInterval(this.timer_heart_beat); }
        this.socket.destroy();
        this.connect_ok = false;
        print_send_event_number();
        return;
    }

    var cid = '18E13000C001';
    var cid_time = (new Date()).Format("yyyy-MM-dd hh:mm:ss");
                               
    this.send('d', this.rollseq, 'event,id:' + this.name + ',' 
              + 'cid:' + cid + ','
              + 'time:' + cid_time);
    
    this.event_number --;
}



for (var i = 0; i < max_connections; i ++) {
    var name = (i+1).toString();
    while ( name.length < 4) {
        name = '0' + name;
    }
    connections.push(new Client(ip, port, name, event_number_per_client));
}

for (var i = 0; i < connections.length; i ++) {
    connections[i].go();
}

Date.prototype.Format = function (fmt) {
    var o = {
        "M+": this.getMonth() + 1,   //月份
        "d+": this.getDate(),        //日
        "h+": this.getHours(),       //小时
        "m+": this.getMinutes(),     //分
        "s+": this.getSeconds(),     //秒
        "q+": Math.floor((this.getMonth() + 3) / 3), //季度
        "S": this.getMilliseconds()   //毫秒
    };
    if (/(y+)/.test(fmt)) fmt = fmt.replace(RegExp.$1, (this.getFullYear() + "").substr(4 - RegExp.$1.length));
    for (var k in o)
        if (new RegExp("(" + k + ")").test(fmt)) fmt = fmt.replace(RegExp.$1, (RegExp.$1.length == 1) ? (o[k]) : (("00" + o[k]).substr(("" + o[k]).length)));
    return fmt;
}

