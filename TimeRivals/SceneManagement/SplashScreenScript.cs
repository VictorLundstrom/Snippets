using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Video;
using System;

public class SplashScreenScript : MonoBehaviour
{
    public VideoPlayer VP;
    private float time = 0;
    bool hasPlayed;
    private void Update()
    {
        time += Time.deltaTime;
        if (time >= 1.95f && !hasPlayed)
        {
            hasPlayed = true;
            VP.loopPointReached += EndReached;
            VP.Play();
        }
    }

    private void EndReached(VideoPlayer vp)
    {
        Destroy(vp.gameObject);
    }
}
