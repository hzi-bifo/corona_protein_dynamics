


def get_time_period(time, period):
	if period == "day":
		return time
	if period == "month":
		month = time.rsplit("-",1)[0]
		return month
	if period == "year":
		year = time.rsplit("-",2)[0]
		return year
