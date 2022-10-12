using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;


public class SplashScreenTransition : MonoBehaviour
{
    [SerializeField] private GameObject _videoPlayer;

    // Update is called once per frame
    void Update()
    {
        if (!_videoPlayer)
        {
            SceneManager.LoadScene((int)SceneIndexes.MANAGER);
        }
    }
}
