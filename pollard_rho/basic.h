struct Time {
    struct timespec tm;
};

using Time_Diff = double;

Time get_time() {
    struct timespec tm = {};
    clock_gettime(CLOCK_MONOTONIC, &tm);

    return {tm};
}

double compute_time_diff(Time t1, Time t2) {
    double diff = 0.0;
    diff += (double)(t2.tm.tv_sec  - t1.tm.tv_sec)  * 1'000'000.0;
    diff += (double)(t2.tm.tv_nsec - t1.tm.tv_nsec) / 1'000.0;
    return diff;
}

