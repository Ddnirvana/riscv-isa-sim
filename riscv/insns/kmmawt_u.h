P_LOOP(32, {
  int64_t mres = (int64_t)ps1 * P_SH(ps2, 1);
  int32_t round = (((mres >> 15) + 1) >> 1);
  bool sat = false;
  pd = (sat_add<int32_t, uint32_t>(pd, round, sat));
  P.VU.vxsat |= sat;
})