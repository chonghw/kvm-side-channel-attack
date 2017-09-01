import os, sys, subprocess


def process():

	for file in os.listdir(sys.argv[1]):

		fullname = os.path.join(sys.argv[1], file)
		ext = os.path.splitext(fullname)[-1]

		if file.find("listen")>=0 and ext != '.result':
			ansfile = fullname.replace("listen", "send")
			subprocess.call(["./analyze", fullname, ansfile])


if __name__ == "__main__":
	process()