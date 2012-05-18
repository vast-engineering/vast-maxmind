var	opt = require('optimist'),
	argv = opt
		.alias('h','help')
		.alias('?','help')
		.describe('help', 'Display help')
		.alias('i','ip')
		.describe('ip', 'Ip address to lookup in the format w.x.y.z')
		.usage('Looks up the given ip address  Ex: node tests/lookup.js -i "8.8.8.8" .\nUsage: $0')
		.argv;

if(argv.help) {
	opt.showHelp();
	return;
}

// Add setTimeout since the module needs time to asynchronously initialize.
setTimeout(function() {

var VastMaxmind = require('../index.js').VastMaxmind,
	vmm = new VastMaxmind('/usr/local/share/GeoIP/GeoIPCity.dat'),
	addr = argv.i;

if (!addr) {
	console.log("No IP specified.  Using 8.8.8.8 which is google dns.");
	addr = "8.8.8.8";
}

vmm.location(addr, function(data) {
	console.log(JSON.stringify(data) );
	process.exit();
});

}, 1000);