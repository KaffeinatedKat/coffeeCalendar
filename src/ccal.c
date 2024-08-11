#include "ccal.h"

int ccal_read_file(struct file *event_list, const char *ccal_file) {
	int fd;
	char *map;
	struct stat st;

	fd = open(ccal_file, O_RDONLY);
	if (fd == -1) {
        return -1;
    }
	if (fstat(fd, &st) == -1) {
        return -1;
    }

	map = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);

	event_list->content = strndup(map, st.st_size);
	event_list->size = st.st_size;

	munmap(map, st.st_size);
	close(fd);

	return 0;
}

int parse_pair(struct file *event_list, const char *tag, char **out) {
	char *p1, *p2;

	p1 = strstr(event_list->content, tag);
	if (p1 == NULL) {
        return -1;
    }
	p1 += strlen(tag);

	p2 = strstr(p1, tag);
	if (p2 == NULL) {
        return -1;
    }

	*out = strndup(p1, p2-p1);

	return 0;
}

int increment_line(struct file *event_list) {
	event_list->content = strchr(event_list->content, '\n');
	if (event_list->content == NULL) {
        return -1;
    }
	event_list->content++;

	return 0;
}


int ccal_add_event(struct ccal_calendar *cal, struct ccal_event event, char **blacklist, int blacklist_size) {
    // Don't add blacklisted events
    for (int x = 0; x < blacklist_size; x++) {
        if (strcmp(event.name, blacklist[x]) == 0) {
            return 0;
        }
    }

	if (cal->nevents == cal->size) {
		if (cal->size == 0) cal->size = 4;
		else cal->size *= 2;
		cal->events = reallocarray(cal->events, cal->size, sizeof(struct ccal_event));
		if (cal->events == NULL) return -1;
	}
	memcpy(cal->events+cal->nevents, &event, sizeof(struct ccal_event));
	cal->nevents++;
	return 0;
}

int date_ascending(void *va, void *vb) {
	struct ccal_event *a = va, *b = vb;
	time_t atime = mktime(&a->date), btime = mktime(&b->date);
	return atime < btime ? -1 : atime > btime;
}

int ccal_calendar_destroy(struct ccal_calendar *cal) {
	free(cal->events);
	return 0;
}

int ccal_get_number_of_events(struct ccal_calendar *cal, struct tm date) {
	int events = 0;
	struct ccal_event *event;
	for (int i = 0; i < cal->nevents; i++) {
		event = &cal->events[i];
		if (event->date.tm_year + 1900 != date.tm_year) continue;
		if (event->date.tm_mon + 1 != date.tm_mon) continue;
		if (event->date.tm_mday != date.tm_mday) continue;

		events++;
	}

	return events;
}

int ccal_get_max_events_for_week(struct ccal_calendar *cal, struct tm date) {
	int tmp, max = 0;
    struct tm new_date = date;
    time_t t;

    new_date.tm_year += 1900;
    new_date.tm_mon += 1;
    new_date.tm_mday += 1;
    t = mktime(&new_date);

	for (int i = 0; i < 7; i++) {
		localtime_r(&t, &date);
		tmp = ccal_get_number_of_events(cal, date);
		max = tmp > max ? tmp : max;
		t += (60 * 60 * 24);
	}

	return max;
}

