#![allow(non_snake_case)]
#![allow(dead_code)]
#![allow(unused_variables)]

mod geometry;
mod rtree;

use crate::geometry::Point;
use crate::geometry::Rectangle;

use crate::rtree::Node;


fn main() {
    println!("Hello, welcome to the NIR-Tree interface.");
    println!("Please select one of:");
    println!("\t- Squares benchmark (s)");
    println!("\t- Cubes benchmark (c)");

    let boundingBoxLeftLeaf = Rectangle::new(0, 0, 13, 21, 273);
    let boundingBoxRightLeaf = Rectangle::new(15, 4, 21, 21, 102);
    let boundingBoxLeft = Rectangle::new(0, 0, 13, 21, 273);
    let boundingBoxRight = Rectangle::new(15, 4, 21, 21, 102);

    let nodeLeft = Node::new(0, vec![boundingBoxLeftLeaf], Vec::new());
    let nodeRight = Node::new(1, vec![boundingBoxRightLeaf], Vec::new());
    let root = Node::new(2, vec![boundingBoxLeft, boundingBoxRight], vec![nodeLeft, nodeRight]);

    println!("{:?}", root.children[0].boundingPolygons[0].lowerLeft);
    println!("{:?}", root.children[1].boundingPolygons[0].lowerLeft);

    let seek = root.searchPoint(&Point{x: 1, y: 1});

    println!("We found point (1, 1) is {}.", seek);
}
