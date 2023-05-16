#!/bin/bash
# Bash script to zip the whole project in order to make it deriverable
# please make sure zip, pv and texlive are installed

OUTFILE=../ssdd_p2_100429021_100383016.zip

# make sure git is up to date
echo "Making sure git is up to date..."
git pull
git checkout main

# compile the report (and save it to root folder)
echo "Compiling the report..."
cd report

pdflatex report.tex -halt-on-error > /dev/null
cp report.pdf ..

cd ..

# remove whatever is in src/.gitignore
echo "Removing files in .gitignore..."
cd src

for f in $(cat .gitignore) ; do 
    rm -rf "$f"
done

make clean

cd ..

# cleanup README.md
start=$(sed -n '/#instalación-y-ejecución/{=; q;}' README.md)  # count lines til start
end=$(($(sed -n '/# Instalación y ejecución/{=; q;}' README.md) - 1))

sed -i "$start","$end"'d' README.md  # remove lines


# zip it (excluding useless stuff)
echo "Zipping..."
zip -r $OUTFILE . -x zip.sh report/\* \*.git\* img/\* LICENSE README.md

# cleanup
echo "Cleaning up..."
git reset --hard HEAD
