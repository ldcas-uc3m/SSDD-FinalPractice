#!/bin/bash
# Bash script to zip the whole project in order to make it deriverable
# please make sure zip, pv and texlive are installed

OUTFILE=../ssdd_p2_100429021_100383016.zip

# make sure git is up to date
# echo "Making sure git is up to date..."
# git pull
# git checkout main

# compile the report (and save it to root folder)
echo "Compiling the report..."
cd report

pdflatex report.tex -halt-on-error > /dev/null
cp report.pdf ..

cd ..

# remove stupid stuff
echo "Cleaning unwanted files..."

cd src
make clean
cd ..


# cleanup README
cp README.md README

start=$(sed -n '/#instalación-y-ejecución/{=; q;}' README)  # count lines til start
end=$(($(sed -n '/# Instalación y ejecución/{=; q;}' README) - 1))

sed -i "$start","$end"'d' README  # remove lines


# zip it (excluding useless stuff)
echo "Zipping..."
zip -r $OUTFILE . -x zip.sh report/\* \*.git\* img/\* LICENSE README.md

# cleanup
echo "Cleaning up..."
rm README
rm report.pdf
