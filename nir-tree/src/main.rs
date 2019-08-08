#![allow(non_snake_case)]

mod rtree;

use crate::rtree::Point;
use crate::rtree::Rectangle;
use crate::rtree::Node;

fn main() {
    println!("Hello, welcome to the NIR-Tree interface.");
    println!("Please select one of:");
    println!("\t- Squares benchmark (s)");
    println!("\t- Cubes benchmark (c)");

    let boundingBoxLeftLeaf = Rectangle{lowerLeft: Point{x: 0, y: 0}, upperRight: Point{x: 13, y: 21}};
    let boundingBoxRightLeaf = Rectangle{lowerLeft: Point{x: 15, y: 4}, upperRight: Point{x: 21, y: 21}};
    let boundingBoxLeft = Rectangle{lowerLeft: Point{x: 0, y: 0}, upperRight: Point{x: 13, y: 21}};
    let boundingBoxRight = Rectangle{lowerLeft: Point{x: 15, y: 4}, upperRight: Point{x: 21, y: 21}};
    let boundingBoxRoot = Rectangle{lowerLeft: Point{x: 0, y: 0}, upperRight: Point{x: 21, y: 21}};

    let nodeLeft = Node{boundingPolygons: vec![boundingBoxLeftLeaf], children: Vec::new()};
    let nodeRight = Node{boundingPolygons: vec![boundingBoxRightLeaf], children: Vec::new()};
    let root = Node{boundingPolygons: vec![boundingBoxLeft, boundingBoxRight], children: vec![nodeLeft, nodeRight]};

    println!("{:?}", root.children[0].boundingPolygons[0].lowerLeft);
    println!("{:?}", root.children[1].boundingPolygons[0].lowerLeft);

    let seek = root.search(&Point{x: 1, y: 1});

    println!("We found point (1, 1) is {}.", seek);
}
