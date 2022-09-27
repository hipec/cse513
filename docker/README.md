# How to install docker
    
You can follow the follwoing links to get the [docker installtion](https://docs.docker.com/engine/install/ubuntu/)
    You can also follow the [post-installtion](https://docs.docker.com/engine/install/linux-postinstall/) steps to get things running
# Why docker
Docker provides us a good abstraction to the enviroment to deploy the code.
Multiple different codebases might require multiple conflicting dependencies. Examples Ubuntu 22.04 uses libc 2.35 which conflicts with argobots deployment. 
Why manage these dependencies when you can  have a environment tailored to you.
Docker is not a VM. It is just wrapper over [cgroups](https://www.nginx.com/blog/what-are-namespaces-cgroups-how-do-they-work/#:~:text=Namespaces%20provide%20isolation%20of%20system,can%20use%20namespaces%20and%20cgroups.) and [namespaces](https://www.nginx.com/blog/what-are-namespaces-cgroups-how-do-they-work/#:~:text=Namespaces%20provide%20isolation%20of%20system,can%20use%20namespaces%20and%20cgroups.) on a linux process (commonly default namespaces is used for all process).


# Basics of Docker

## Basic Workflow.
   Docker provides us with multiple images for work environment that we can choose from, if none fit we create a new environment using the given docker images.

  We use **docker images** to create **docker containers**. If we dont have appropriate docker images, we generate a **docker file** to create our own **docker image**.

### Docker Images

   These work as a blueprint for our environment which will be generated. Everytime we want the environment(container). Images are templates which can be quickly deployed. Examples of images we can use can be found on [dockerhub](https://hub.docker.com/)

### Dockerfile
    
   This file is useful to create docker images. If we have no docker image that satifies our needs. This takes time to generate a docker image as it needs to fetch all the dependencies and install them and then package them in docker image. [Tutorial on how to write a docker file](https://takacsmark.com/dockerfile-tutorial-by-example-dockerfile-best-practices-2018/) 

### Docker Containers
   This is the work space we get when we run a docker image. 
   
   Depending upon the image it can start a http server or can give you a bash terminal to interact with. 
   
   **Once the docker container is closed we loose all the data stored inside the container**. We need to mount storage inside docker if we want any state. Or you can create a image from the docker container itself this is not advised. We keep our docker containers stateless

# Docker  

## Creating Docker Image from Dockerfile
   ```bash
    git clone https://github.com/hipec/cse513.git
    cd cse513/docker
    docker build -t argobots . 
   ```
   **docker build** is used to create a docker image of name argobots:latest using the Dockerfile(context) in the given folder.
    
   Once we have generated the docker image we can create a docker container out which will provide us with a container with binaries of Ubuntu:18.04 and an argobots install.
    You can also add environment variable while you define the dockerfile which will be added to the docker image

## Creating a Docker Container for Docker Image
   ```bash
    sh run_docker.sh
   ```
   
   run_docker.sh contains the following things
   ```bash
    #bin/bash
    mkdir data/
    sudo docker run --privileged=true -it \
	    -v "$(pwd)"/data:/data/ \
	argobots
   ```
   The script generates a folder named data which will be mounted to the docker container. All the state/updates should be stored in the data folder inside docker. 
   **--privileged=true **is needed because we loose rights to access the folder inside docker container
   **  -it** tells the run docker in interactive mode
   **-v** mounts the $(pwd)/data host folder to /data container folder. (This can be done in much nicer manner using volumes which is remove the need of using priveleged=true)
   **argbots** is the name of the image

   You can change the bash script and update which folder we want to pass through.
    
## Docker vs baremetal results
