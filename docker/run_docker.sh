#bin/bash
mkdir data/
sudo docker run --privileged=true -it \
	-v "$(pwd)"/data:/data/ \
	argobots