bazel build ps

rm -rf ps
ln -s ../bazel-bin/ps-lite/ps .
