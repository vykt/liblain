#!/bin/bash

#
# This is stitched together from chatGPT, not exactly efficient >_>
#

src_dir="markdown"
dest_dir="man.3"

#check that pandoc is installed
if ! command -v pandoc &> /dev/null; then
    echo "error: pandoc is not installed"
    exit 1
fi

#create the dest dir if it doesn't exist
mkdir -p ${dest_dir}

#convert all markdown files to roff
for file in "$src_dir"/*.md; do
    if [ -f "$file" ]; then
        filename=$(basename "$file" .md)
        dest_file="$dest_dir/$filename.3"

        echo "converting $file to $dest_file..."
        pandoc -s -t man "$file" -o "$dest_file"
        if [ $? -ne 0 ]; then
            echo "error: pandoc failed to convert $file"
        fi
    fi
done

#remove incorrect formatting
for file in "$dest_dir"/*; do
    if [[ -f "$file" ]]; then
        sed -i '/.TH "" "" "" ""/d' "$file"
    fi
done

#add title and name to each file
for file in "$dest_dir"/*; do

  basefile=$(basename "$file")
  namefile=${basefile%.*}
  filename=$(echo $namefile | tr [A-Z] [a-z])
  FILENAME=$(echo $namefile | tr [a-z] [A-Z])

  # add the filename to the beginning of the file in all uppercase
  echo ".TH ${FILENAME} 3 \"June 2023\" \"libpwu 1.0\" \"${filename}\"" | cat - $file > temp && mv temp $file

  # add the filename to the beginning of the file in all lowercase
  echo ".IX Title \"${FILENAME} 3"  | cat - $file > temp && mv temp $file

  new_filename="libpwu_$basefile"
  mv "$dest_dir"/"$basefile" "$dest_dir"/"$new_filename"

done


echo "done!"
