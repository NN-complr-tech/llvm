

int f(int a, int b) {
  int c = 0;
  c++;
  c++;
  c++;
  a = c;
  c--;
  c--;
  b = c;
  c++;
  return  a + b + c;
}