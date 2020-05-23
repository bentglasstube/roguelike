class PRNG {
  constructor(seed) {
    this.state = seed;
  }

  value() {
    var t = this.state += 0x6d2b79f5;
    t = Math.imul(t ^ t >>> 15, t | 1);
    t ^= t + Math.imul(t ^ t >>> 7, t | 61);
    return ((t^t >>> 14) >>> 0) / 4294967296;
  }

  below(max) {
    return Math.floor(this.value() * max);
  }

  range(min, max) {
    return Math.floor(this.value() * (max - min)) + min;
  }

  element(list) {
    return list[this.below(list.length)];
  }
}
