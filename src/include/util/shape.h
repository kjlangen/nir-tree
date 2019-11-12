#ifndef __ISHAPE__
#define __ISHAPE__

class IShape
{
	public:
		virtual bool intersects(IShape &s) = 0;
		virtual bool contains(IShape &s) = 0;
		virtual IShape() = 0;
		virtual ~IShape() = 0;
};