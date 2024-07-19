#include "ccal.h"

int read_file(struct file *event_list, const char *ccal_file) {
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


int add_event(struct calendar *cal, struct event event) {
	if (cal->nevents == cal->size) {
		if (cal->size == 0) cal->size = 4;
		else cal->size *= 2;
		cal->events = reallocarray(cal->events, cal->size, sizeof(struct event));
		if (cal->events == NULL) return -1;
	}
	memcpy(cal->events+cal->nevents, &event, sizeof(struct event));
	cal->nevents++;
	return 0;
}

int date_ascending(const void *va, const void *vb) {
	struct event *a = va, *b = vb;
	time_t atime = mktime(&a->date), btime = mktime(&b->date);
	return atime < btime ? -1 : atime > btime;
}

int calendar_create(struct calendar *cal, char *file) {
	char *out;
	struct event event = {0}; // A single parsed event
	struct file event_list; // The list of all events to be parsed
	read_file(&event_list, file);

	do {
		if (parse_pair(&event_list, "%N", &out) != -1) {
			event.name = out;
		}
		if (parse_pair(&event_list, "%AD", &out) != -1) {
			if (strcmp(out, "No") == 0) event.all_day = 0;
			if (strcmp(out, "Yes") == 0) event.all_day = 1;
			free(out);
		}
		if (parse_pair(&event_list, "%D", &out) != -1) {
			strptime(out, "%Y/%m/%d", &event.date);
			free(out);
		}
		if (parse_pair(&event_list, "%B", &out) != -1) {
			strptime(out, "%H:%M", &event.start);
			free(out);
		}
		if (parse_pair(&event_list, "%E", &out) != -1) {
			strptime(out, "%H:%M", &event.end);
			free(out);
		}
		if (parse_pair(&event_list, "%C", &out) != -1) {
			event.cal_name = out;
		}
		add_event(cal, event);
		memset(&event, 0, sizeof(struct event));
	} while (increment_line(&event_list) != -1);

	qsort(cal->events, cal->nevents, sizeof(struct event), date_ascending);
	free(event_list.content);

	return 0;
}

int calendar_destroy(struct calendar *cal) {
	struct event *event;
	for (int x = 0; x < cal->nevents; x++) {
		event = &cal->events[x];
		free(event->cal_name);
		free(event->name);
	}
	free(cal->events);

	return 0;
}

int get_number_of_events(struct calendar *cal, struct tm date) {
	int events = 0;
	struct event *event;
	for (int i = 0; i < cal->nevents; i++) {
		event = &cal->events[i];
		if (event->date.tm_year < date.tm_year) continue; \
		else if (event->date.tm_year > date.tm_year) break;
		if (event->date.tm_mon < date.tm_mon) continue; \
		else if (event->date.tm_mon > date.tm_mon) break;
		if (event->date.tm_mday < date.tm_mday) continue; \
		else if (event->date.tm_mday > date.tm_mday) break;

		events++;
	}
	return events;
}

int get_max_events_for_week(struct calendar *cal, int16_t year, int8_t month, int8_t day) {
	int tmp, max = 0;
	struct tm date = {
		.tm_year = year - 1900,
		.tm_mon = month,
		.tm_mday = day,
	};
	time_t t = mktime(&date);
	t -= date.tm_wday * (60 * 60 * 24);

	for (int i = 0; i < 7; i++) {
		localtime_r(&t, &date);
		tmp = get_number_of_events(cal, date);
		max = tmp > max ? tmp : max;
		t += (60 * 60 * 24);
	}

	return max;
}

