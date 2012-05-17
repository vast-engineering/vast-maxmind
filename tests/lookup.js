var VastMaxmind = require('../build/Release/vastmaxmind').VastMaxmind,
	vmm = new VastMaxmind('/usr/local/share/GeoIP/GeoIPCity.dat'),
	addr = "8.8.8.8"; // 1029177344;
	//addr = 1029177344;

debugger;

var foo = vmm.location(addr, function(data) {
	console.log(JSON.stringify(data) );
	process.exit();
});


// var bad = new VastMaxmind('ksadfhjkdh');
// var poo = vmm.location(addr, function(data) {
// 	console.log(JSON.stringify(data) );
// 	process.exit();
// });