var tcslib = require('./tcs3414cs');

tcslib.init();


// Print out the r, g, b, and clr value every 0.5 seconds
var myInterval = setInterval(function()
{
	var rgb = tcslib.readRGB();
	console.log(rgb.r + ", " + rgb.g + ", " + rgb.b + ", " + rgb.clr);
}, 500);

// Print message when exiting
process.on('SIGINT', function()
{
	clearInterval(myInterval);
	console.log("Exiting...");
	process.exit(0);
});

