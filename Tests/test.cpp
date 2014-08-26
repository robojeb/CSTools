void foo(){
  int x;
  double NotOkay;
  float thisIsOkay;
}

class test {
  int thing_;
  static const int static_;

  void bar();

};

void test::bar()
{
  thing_ += 1;
}

template <typename T>
T mul2(T x) {
  return x*2;
}
