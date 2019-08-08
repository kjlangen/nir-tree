#![allow(non_snake_case)]

extern crate rand;

use rand::prelude::*;
use std::env;

struct Square(u64, u64, u64);
struct Cube(u64, u64, u64, u64);

fn generateCubes(maxSide: u64, maxX: u64, maxY: u64, maxZ: u64, n: u64) -> Vec<Cube>
{
	let mut rng = thread_rng();
	let sideDist = rand::distributions::Uniform::new_inclusive(0, maxSide);
	let xDist = rand::distributions::Uniform::new_inclusive(0, maxX);
	let yDist = rand::distributions::Uniform::new_inclusive(0, maxY);
	let zDist = rand::distributions::Uniform::new_inclusive(0, maxZ);

	let mut cubes = vec![Cube(rng.sample(xDist), rng.sample(yDist), rng.sample(zDist), rng.sample(sideDist))];

	for i in 1..n
	{
		cubes.push(Cube(rng.sample(xDist), rng.sample(yDist), rng.sample(zDist), rng.sample(sideDist)));
	}

	return cubes;
}

fn generateSquares(maxSide: u64, maxX: u64, maxY: u64, n: u64) -> Vec<Square>
{
	let mut rng = thread_rng();
	let sideDist = rand::distributions::Uniform::new_inclusive(0, maxSide);
	let xDist = rand::distributions::Uniform::new_inclusive(0, maxX);
	let yDist = rand::distributions::Uniform::new_inclusive(0, maxY);

	let mut squares = vec![Square(rng.sample(xDist), rng.sample(yDist), rng.sample(sideDist))];

	for i in 1..n
	{
		squares.push(Square(rng.sample(xDist), rng.sample(yDist), rng.sample(sideDist)));
	}

	return squares;
}

fn main()
{
	// Parse the command line args
	let args: Vec<String> = env::args().collect();
	assert!(args.len() == 6 || args.len() == 7);
	let choice: String = match args[1].parse()
	{
		Ok(String) => String,
		_ => panic!("choice is not a string")
	};
	let maxSide: u64 = match args[2].parse()
	{
		Ok(u64) => u64,
		_ => panic!("maxSide not a u64")
	};
	let maxX: u64 = match args[3].parse()
	{
		Ok(u64) => u64,
		_ => panic!("maxX not a u64")
	};
	let maxY: u64 = match args[4].parse()
	{
		Ok(u64) => u64,
		_ => panic!("maxY not a u64")
	};

	// Generate squares or cubes
	if choice == "squares"
	{
		let n: u64 = match args[5].parse()
		{
			Ok(u64) => u64,
			_ => panic!("n not a u64")
		};

		let generatedSquares: Vec<Square> = generateSquares(maxSide, maxX, maxY, n);

		for i in 0..generatedSquares.len()
		{
			println!("Square {}: (x, y) = ({}, {}), extent = {}", i, generatedSquares[i].0, generatedSquares[i].1, generatedSquares[i].2);
		}
	}
	else if choice == "cubes"
	{
		let maxZ: u64 = match args[5].parse()
		{
			Ok(u64) => u64,
			_ => panic!("maxZ not a u64"),
		};
		let n: u64 = match args[6].parse()
		{
			Ok(u64) => u64,
			_ => panic!("n not a u64")
		};
		
		let generatedCubes: Vec<Cube> = generateCubes(maxSide, maxX, maxY, maxZ, n);

		for i in 0..generatedCubes.len()
		{
			println!("Cube {}: (x, y, z) = ({}, {}, {}), extent = {}", i, generatedCubes[i].0, generatedCubes[i].1, generatedCubes[i].2, generatedCubes[i].3);
		}
	}
}
