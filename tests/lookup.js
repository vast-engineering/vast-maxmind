var VastMaxmind = require('../build/Release/vastmaxmind').VastMaxmind,
	vmm = new VastMaxmind('/usr/local/share/GeoIP/GeoIPCity.dat'),
	addr = "8.8.8.8";

var foo = vmm.location(addr, function(data) {
	console.log(JSON.stringify(data) );
	process.exit();
});