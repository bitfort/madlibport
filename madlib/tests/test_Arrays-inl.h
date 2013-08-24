

int byte_arr() {
  char arr[16];
  dbconnector::mainmem::ByteString model(arr, 16);
  EXPECT_EQ(model.size(), 16);
  arr[0] = 7;
  arr[15] = 19;
  EXPECT_EQ(model[0], 7);
  EXPECT_EQ(model[15], 19);
  return 1;
}

int mutable_byte_arr() {
  char arr[16];
  dbconnector::mainmem::MutableByteString model(arr, 16);
  model[0] = 7;
  model[15] = 19;
  EXPECT_EQ(arr[0], 7);
  EXPECT_EQ(arr[15], 19);
  return 1;
}

void TEST_Arrays() {
  RUNTEST(byte_arr)
  RUNTEST(mutable_byte_arr)
}

