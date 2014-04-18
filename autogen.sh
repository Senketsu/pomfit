echo "Regenerating autotools files"
autoreconf --install || exit 1
echo "Running configure scrips"
./configure
