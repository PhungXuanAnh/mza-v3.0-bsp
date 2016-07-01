/*
 * Driver for CBC34803 RTC.
 *
 * Copyright (C) 2009 Semihalf.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/rtc.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/bcd.h>

#define CBC34803_SECONDS_MASK	0x7F	/* Mask over seconds value */

#define CBC34803_MINUTES_MASK	0x7F	/* Mask over minutes value */

#define CBC34803_HOURS_MASK	0x3F	/* Mask over hours value */

struct cbc34803_regs {
	uint8_t		seconds;
	uint8_t		minutes;
	uint8_t		cent_hours;
	uint8_t		date;
	uint8_t		month;
	uint8_t		years;
};

static struct i2c_driver cbc34803_driver;

static int cbc34803_read(struct device *dev, void *data, uint8_t off, uint8_t len) // ham chay thu 3 cua driver.
{	printk("hibk, cbc34803_read\n");
	struct i2c_client *client = to_i2c_client(dev);
	struct i2c_msg msgs[] = {// transaction with start.
		{
			.addr = client->addr, // address of cbc34803
			.flags = 0,
			.len = 1,// numbers of data bytes in buf being read  from or write to cbc34803
			.buf = &off, // chua du lieu ve dia chi cua thanh ghi giay.
		}, {
			.addr = client->addr,// address of cbc34803		
			.flags = I2C_M_RD, // mode read: doc du lieu tu slave to master.
			.len = len,        //  1 byte
			.buf = data, // du lieu doc ve se ghi vao bien regs.

		}
	};

	if (i2c_transfer(client->adapter, msgs, 2) == 2)
		return 0;

	return -EIO;
}

static int cbc34803_write(struct device *dev, void *data, uint8_t off, uint8_t len) // ham chay thu4 cua driver.
{	printk("hibk, cbc34803_write\n");
	struct i2c_client *client = to_i2c_client(dev);
	uint8_t buffer[len + 1];

	buffer[0] = off;
	memcpy(&buffer[1], data, len);

	if (i2c_master_send(client, buffer, len + 1) == len + 1)
		return 0;

	return -EIO;
}

static int cbc34803_rtc_read_time(struct device *dev, struct rtc_time *tm) // ham chay thu 5
{
	struct cbc34803_regs regs;
	int error;

	error = cbc34803_read(dev, &regs, 1, sizeof(regs));

	if (error)
		return error;
	tm->tm_sec = bcd2bin(regs.seconds & CBC34803_SECONDS_MASK);
	tm->tm_min = bcd2bin(regs.minutes & CBC34803_SECONDS_MASK);
	tm->tm_hour = bcd2bin(regs.cent_hours & CBC34803_HOURS_MASK);
	tm->tm_mday = bcd2bin(regs.date);
	tm->tm_mon = bcd2bin(regs.month) - 1;
	tm->tm_year = bcd2bin(regs.years) + 100;
				

	return rtc_valid_tm(tm);
}

static int cbc34803_rtc_set_time(struct device *dev, struct rtc_time *tm) // chay khi cai dat time tu android.
{
	struct cbc34803_regs regs;

	regs.seconds = bin2bcd(tm->tm_sec);
	regs.minutes = bin2bcd(tm->tm_min);
	regs.cent_hours = bin2bcd(tm->tm_hour);
	regs.date = bin2bcd(tm->tm_mday);
	regs.month = bin2bcd(tm->tm_mon + 1);

	if (tm->tm_year >= 100) {
		regs.years = bin2bcd(tm->tm_year - 100);
	} else
		regs.years = bin2bcd(tm->tm_year);

	return cbc34803_write(dev, &regs, 1, sizeof(regs));
}

static const struct rtc_class_ops cbc34803_rtc_ops = {
	.read_time	= cbc34803_rtc_read_time,
	.set_time	= cbc34803_rtc_set_time,
};

static int cbc34803_probe(struct i2c_client *client,
				const struct i2c_device_id *id)// ham chay thu 2 cua driver
{	printk("hibk, runcbc34803_probe\n");
	struct device *dev = &client->dev;
	struct rtc_device *rtc;
	uint8_t reg;
	int error;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
		return -ENODEV;

	rtc = rtc_device_register(cbc34803_driver.driver.name, &client->dev,
						&cbc34803_rtc_ops, THIS_MODULE);
	if (IS_ERR(rtc))
		return PTR_ERR(rtc);

	i2c_set_clientdata(client, rtc);

	return 0;
}

static int __devexit cbc34803_remove(struct i2c_client *client)
{	printk("hibk, run cbc34803_remove\n");
	struct rtc_device *rtc = i2c_get_clientdata(client);

	rtc_device_unregister(rtc);
	return 0;
}

static const struct i2c_device_id cbc34803_id[] = {
	{ "hibk", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, cbc34803_id);

static struct i2c_driver cbc34803_driver = {
	.driver = {
		.name	= "cbc34803",
		.owner	= THIS_MODULE,
	},
	.probe		= cbc34803_probe,
	.remove		= __devexit_p(cbc34803_remove),
	.id_table	= cbc34803_id,
};

static __init int cbc34803_init(void) // ham chay dau tien cua 1 driver.
{
	printk("hibk, run cbc34803_init\n");
	return i2c_add_driver(&cbc34803_driver);
}
module_init(cbc34803_init);

static __exit void cbc34803_exit(void)
{	printk("hibk, run cbc34803_exit\n");
	i2c_del_driver(&cbc34803_driver);
}
module_exit(cbc34803_exit);

MODULE_AUTHOR("Bkav");
MODULE_DESCRIPTION("CBC34803 I2C RTC driver");
MODULE_LICENSE("GPL");
