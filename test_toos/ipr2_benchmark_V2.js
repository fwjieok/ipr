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

process.on('SIGINT', function () {
    console.log("");
    process.exit(-1);
});

function Client(ip, port, name, event_number) {
    this.ip           = ip;
    this.port         = port;
    this.name         = 'FFFF' + name;
    this.event_number = event_number;
    this.recv_ack     = false;
    this.rollseq      = 0;
}

Client.prototype.go = function () {
    
    this.socket = net.connect(this.port, this.ip, this.on_connected.bind(this));
    //this.socket.setTimeout(10 * 1000);
    this.socket.on('timeout', this.on_timeout.bind(this));
    this.socket.on("data",    this.on_data.bind(this));
    this.socket.on("error",   this.on_error.bind(this));
    this.socket.on("end",     this.on_end.bind(this));
    this.socket.on("close",   this.on_close.bind(this));
}

Client.prototype.on_connected = function () {
    console.log('[connected],' + (new Date()).format() + ',' + this.name + ',connected');
    
    if (!this.timer_heart_beat) {
        this.timer_heart_beat = setInterval(this.send_heart_beat.bind(this), (Math.round(5 * Math.random()) + heart_beat_time) * 1000);
    }

    if (!this.timer_event) {
        this.timer_event = setInterval(this.send_event.bind(this), event_time_delay * 1000);
    }
}

Client.prototype.on_timeout = function(data) {
    console.log('[ack_timeout],' + (new Date()).format() + ',' + this.name + ',connect timeout');
}

Client.prototype.on_error = function(error) {
    console.log('[error],' + (new Date()).format() + ',' + this.name + ',' + error);   
}

Client.prototype.on_end = function() {
    console.log('[end],' + (new Date()).format() + ',' + this.name);   
    if (this.timer_heart_beat) {
        clearInterval(this.timer_heart_beat);
    }
    if (this.timer_event) {
        clearInterval(this.timer_event);
    }

    this.socket.destroy();
}

Client.prototype.on_close = function() {
    console.log('[close],' + (new Date()).format() + ',' + this.name);
    
    if (this.timer_heart_beat) {
        clearInterval(this.timer_heart_beat);
    }
    if (this.timer_event) {
        clearInterval(this.timer_event);
    }

    //重连
    setTimeout(this.go.bind(this), 5 * 1000);

}

Client.prototype.on_data = function(data) {
    var pkg   = data.toString();
    var index = pkg.indexOf('ack');
    if (index >= 0) {
        this.rollseq = ++this.rollseq % 10000;
        pkg = pkg.substring(index);
        console.log('[recv],'+(new Date()).format() + ',' + this.name + ',' + pkg);
        this.recv_ack = true;
    }
}

Client.prototype.ack_timeout = function() {
    if (this.recv_ack === false) {
        console.log('[ack_timeout],'+(new Date()).format() + ',' + this.name);
    }
}

Client.prototype.send = function(package_type, rollseq, package_data) {
    var data = package_type + ',' + rollseq + ',' + package_data;
    var pkg  = data + '\r\n';
    console.log('[send],'+(new Date()).format() + ',' + this.name + ',' + data);
    this.socket.write(pkg);
}

Client.prototype.send_heart_beat = function() {
    if (this.rollseq == 0) {
        //握手包
        this.send('d', this.rollseq, 'hi,id:' + this.name + " 0000" + this.name);
    } else {
        //心跳包
        var time = (new Date()).format();
        this.send('d', this.rollseq, 'hi,id:' + this.name + ','
                  + 'status:open,sn:TEST-53R-P5-NG9,time:'+ time +',level:0,ver:14');
        this.recv_ack = false;
        setTimeout(this.ack_timeout.bind(this), 15*3000);
    }
}

Client.prototype.send_event = function() {
    if (this.event_number <= 0) {
        if (this.timer_event) { clearInterval(this.timer_event); }
        return;
    }

    var cid = '18E13000C001';
    var cid_time = (new Date()).format('yyyy-MM-dd,hh:mm:ss');
                               
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

Date.prototype.format = function (fmt) {
    var o = {
        "M+": this.getMonth() + 1,   //月份
        "d+": this.getDate(),        //日
        "h+": this.getHours(),       //小时
        "m+": this.getMinutes(),     //分
        "s+": this.getSeconds(),     //秒
        "q+": Math.floor((this.getMonth() + 3) / 3), //季度
        "S": this.getMilliseconds()   //毫秒
    };
    if (fmt === null || fmt === undefined || fmt === "") {
        fmt = 'yyyy-MM-dd,hh:mm:ss';
    }
    if (/(y+)/.test(fmt)) fmt = fmt.replace(RegExp.$1, (this.getFullYear() + "").substr(4 - RegExp.$1.length));
    for (var k in o)
        if (new RegExp("(" + k + ")").test(fmt)) fmt = fmt.replace(RegExp.$1, (RegExp.$1.length == 1) ? (o[k]) : (("00" + o[k]).substr(("" + o[k]).length)));
    return fmt;
}

