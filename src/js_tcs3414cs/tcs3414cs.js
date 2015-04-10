var libmraa = require('mraa'); //require mraa

var ADDR = 0x39; // device address

var REG_CTL = 0x80;
var REG_TIMING = 0x81;
var REG_INT = 0x82;
var REG_INT_SOURCE = 0x83;
var REG_ID = 0x84;
var REG_GAIN = 0x87;
var REG_LOW_THRESH_LOW_BYTE = 0x88;
var REG_LOW_THRESH_HIGH_BYTE = 0x89;
var REG_HIGH_THRESH_LOW_BYTE = 0x8A;
var REG_HIGH_THRESH_HIGH_BYTE = 0x8B;
var REG_BLOCK_READ = 0xCF;
var REG_GREEN_LOW = 0xD0;
var REG_GREEN_HIGH = 0xD1;
var REG_RED_LOW = 0xD2;
var REG_RED_HIGH = 0xD3;
var REG_BLUE_LOW = 0xD4;
var REG_BLUE_HIGH = 0xD5;
var REG_CLEAR_LOW = 0xD6;
var REG_CLEAR_HIGH = 0xD7;
var CTL_DAT_INIITIATE = 0x03;
var CLR_INT = 0xE0;

/* Timing Register */
var SYNC_EDGE = 0x40;
var INTEG_MODE_FREE = 0x00;
var INTEG_MODE_MANUAL =  0x10;
var INTEG_MODE_SYN_SINGLE = 0x20;
var INTEG_MODE_SYN_MULTI = 0x30;

var INTEG_PARAM_PULSE_COUNT1 = 0x00;
var INTEG_PARAM_PULSE_COUNT2 = 0x01;
var INTEG_PARAM_PULSE_COUNT4 = 0x02;
var INTEG_PARAM_PULSE_COUNT8 = 0x03;

/* Interrupt Control Register */
var INTR_STOP = 40;
var INTR_DISABLE = 0x00;
var INTR_LEVEL = 0x10;
var INTR_PERSIST_EVERY = 0x00;
var INTR_PERSIST_SINGLE = 0x01;

/* Interrupt Souce Register */
var INT_SOURCE_GREEN = 0x00;
var INT_SOURCE_RED = 0x01;
var INT_SOURCE_BLUE = 0x10;
var INT_SOURCE_CLEAR = 0x03;

/* Gain Register */
var GAIN_1 = 0x00;
var GAIN_4 = 0x10;
var GAIN_16 = 0x20;
var GANI_64 = 0x30;
var PRESCALER_1 = 0x00;
var PRESCALER_2 = 0x01;
var PRESCALER_4 = 0x02;
var PRESCALER_8 = 0x03;
var PRESCALER_16 = 0x04;
var PRESCALER_32 = 0x05;
var PRESCALER_64 = 0x06;

var HIGH = 1;
var LOW = 0;


var I2Csensor = new libmraa.I2c(0);


module.exports = {
	init: function()
	{
		// Set timing register
		queueManager.addItem(
			function(){I2Csensor.writeReg(REG_TIMING, INTEG_MODE_FREE)},
			100
		);

		// Set interrupt source register
		queueManager.addItem(
			function(){I2Csensor.writeReg(REG_INT_SOURCE, INT_SOURCE_GREEN)},
			100
		);

		// Set interrupt control register
		queueManager.addItem(
			function(){I2Csensor.writeReg(REG_INT, INTR_LEVEL | INTR_PERSIST_EVERY)},
			100
		);

		// Set gain
		queueManager.addItem(
			function(){I2Csensor.writeReg(REG_GAIN, GAIN_1 | PRESCALER_4)},
			100
		);
		// Enable ADC
		queueManager.addItem(
			function(){I2Csensor.writeReg(REG_CTL, CTL_DAT_INIITIATE)},
			100
		);
	},
	readRGB: function (rgb)
	{
		var buffer = module.exports.i2cReadReg_N (REG_BLOCK_READ, 8);

		// We need 7 bytes of data.
		if (buffer.length > 7)
		{
			var bufferStr = buffer.toString('hex');
			var rgbObj = {};
			rgbObj.g = getColorVal(bufferStr.slice(0, 2), bufferStr.slice(2, 4));
			rgbObj.r = getColorVal(bufferStr.slice(4, 6), bufferStr.slice(6, 8));
			rgbObj.b = getColorVal(bufferStr.slice(8, 10), bufferStr.slice(10, 12));
			rgbObj.clr = getColorVal(bufferStr.slice(12, 14), bufferStr.slice(14, 16));
			return rgbObj;
		}
		else
			exit("buffererror");
	},

	clearInterrupt: function()
	{
		var error = 1;

		if (I2Csensor == NULL)
			exit("initerror");

		error = mraa_i2c_address (m_i2Ctx, ADDR);
		error = mraa_i2c_write_byte (m_i2Ctx, CLR_INT);

		if (error != MRAA_SUCCESS)
			exit("interrupterror");
	},

	i2cReadReg_N: function(reg, len) {
		var readByte = 0;

		if (I2Csensor == null)
			exit("initerror");

		I2Csensor.address(ADDR);
		I2Csensor.writeByte(reg);

		I2Csensor.address(ADDR);
		readByte = I2Csensor.read(len);
		return readByte;
	},
	//function i2cWriteReg_N (uint8_t reg, unsigned int len, uint8_t * buffer) {
	i2cWriteReg_N: function( reg, len, buffer) {
		var error = 1;

		if (I2Csensor == NULL)
			exit("initerror");

		error = I2Csensor.address(ADDR);
		error = I2Csensor.writeByte(reg);
		error = I2Csensor.write(buffer, len);

		return error;
	},
	//TCS3414CS::i2cWriteReg (uint8_t reg, uint8_t data) {
	i2cWriteReg: function(reg, data) {
		//mraa_result_t error = MRAA_SUCCESS;
		var error = 1;

		if (I2Csensor == NULL)
			exit("initerror");

		error = I2Csensor.address(ADDR);
		error = I2Csensor.writeByte(reg);
		error = I2Csensor.writeByte(data);

		return error;
	}
}

// private, non-exported functions
var queueManager = new function()
{
    // run a sensor function in msec milliseconds
    this.addItem = function(func, msec)
    {
        m_queue.push([func, msec]);
        if (!m_inprogress)
            this.processItem();
    }
    this.processItem = function()
    {
        var newitem = m_queue.shift();
        m_inprogress= true
        setTimeout(newitem[0], function()
        {
            (newitem[1])();
            if (m_queue.length > 0)
                queueManager.processItem();
            else
                m_inprogress= false
        });
    }
    var m_interval;
    var m_queue = [];
    var m_inprogress = false;
};

function getColorVal(valStr1, valStr2)
{
	return parseInt("0x" + valStr2) * 256 + parseInt("0x" + valStr1);
}

function exit(msg)
{
	if (msg == "initerror")
		console.log("Couldn't find initilized I2C.");
	else if (msg == "interrupterror")
		console.log("Couldn't clear interrupt.");
	else if (msg == "buffererror")
		console.log("Incorrect data from color sensor.");
	process.exit(0);
}
