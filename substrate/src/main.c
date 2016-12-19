int testSubroutine() {
	return 0x69690000;
}

int _start() {
	return testSubroutine() | 0x6969;
}
