// Dump of the code needed to display the wonder animation

// Wonder Animation
const string WONDER_FP = IMAGES_DIR + "wonder/"; // Just append frame numbers, starting with 0
const int WONDER_FRM_CNT=24;
Frames* wonderFrames = new Frames(WONDER_FP, WONDER_FRM_CNT);


case ANIM_1:
	{
		static int frameNum = 0;
		static int DELAY_USEC = 0.25e6;
		if (screenChange) // Initial events
		{
			screenChange = false;
			frameNum = 0;
		}

		offscreen->Clear();
		int imgWidth = wonderFrames->getImgWidth(frameNum);

		wonderFrames->draw(frameNum, offscreen, M_WIDTH/2 - imgWidth/2, 0);

		offscreen = matrix->SwapOnVSync(offscreen, 1);
		flushBuffAtEnd = false;
		refreshScreen = true;

		usleep(DELAY_USEC);
		if (frameNum > 0) // Cycle through frames backwards (CW spin)
		{
			frameNum--;
		}
		else
		{
			frameNum = WONDER_FRM_CNT-1;
		}
		
	}
		break;

        