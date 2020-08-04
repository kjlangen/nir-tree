#!/bin/bash

cd $1
mkdir -p thumbnails/
echo "Archiving..."
tar -cf PencilVisualization content.xml page_*.xml thumbnails/
echo "Done."
echo "Compressing..."
gzip PencilVisualization
echo "Done."
echo "Renaming..."
mv PencilVisualization.gz PencilVisualization.epgz
echo "Done."
rm -f PencilVisualization
echo "Conversion Finished."
