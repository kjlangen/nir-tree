extern crate rand;

use rand::prelude::*;

fn main()
{
	for i in 0..100
	{
		for j in 0..100
		{
			let x: u8 = random();
			let y: u8 = random();
			println!("({},{})", x, y);
		}
	}
}
