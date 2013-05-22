var _ = require('lodash'),
	VastMaxmind = require('../index.js').VastMaxmind,
	vmm = new VastMaxmind('/usr/local/share/GeoIP/GeoIPCity.dat'),  // local dev
	tap = require('tap'),
	test = tap.test;


var ops = [
	{"ip":"8.8.8.8","ipnum":134744072,"country":"US","state":"CA","city":"Mountain View","zip":"94043","latitude":37.4192008972168,"longitude":-122.05740356445312,"areacode":650}, 
	{"ip":"208.67.222.222","ipnum":3494108894,"country":"US","state":"CA","city":"San Francisco","zip":"94107","latitude":37.76969909667969,"longitude":-122.39330291748047,"areacode":415}, 
	{"ip":"12.249.215.38","ipnum":217700134,"country":"US","state":"TX","city":"Austin","zip":"N/A","latitude":30.267200469970703,"longitude":-97.74310302734375,"areacode":512},
	{"ip":"174.129.212.2","ipnum":2927744002,"country":"US","state":"VA","city":"Ashburn","zip":"N/A","latitude":39.043701171875,"longitude":-77.48750305175781,"areacode":703}
];

_.each(ops, function(op) {

	test("Lookup " + op.ip, function(t) {

		vmm.location(op.ip, function(data) {
			t.deepEqual(data, op, "Output should match");
			t.end();
		});
	});

});






