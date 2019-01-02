# Car-Counter---Image-Processing-Computer-Vision-project
The program takes in a video file and idividually analyses two frames of images per iteration and compares them to track the movement
of objects (cars)
Each image is converted to greyscale to obtain better contrast of the changes between the images
morphological operations of dilation and erosion and performed to distinguish blobs more clearly.
Features such as gaussian blur and thresholding is also used to identify blobs by smoothing out the differences and filtering out
the colors in the lower greyscale spectrum.
The program keeps track of the blobs in the video using vectors.
It uses a predictive algorithm to predict the position (trajectory) of blobs and compares these position with the actual blob positions to determine if new cars appear on video or to keep track of existing cars on the video frame.
Using the these functions, the program successfully keeps count of the number of cars passing by in the video.
