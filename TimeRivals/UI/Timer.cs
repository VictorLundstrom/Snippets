using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using TMPro;

public class Timer : MonoBehaviour
{
    private Animator _uiAnimator;

    [SerializeField] private TimerSceneTransition _timerSceneTransition;

    public TextMeshProUGUI TimeText;
    public float CountdownTime = -1;
    public float RoundTime = -1;
    private bool _roundTimeShouldRun; //False == countdowntimer, True == roundtimer
    public bool RoundTimeShouldRun { get { return _roundTimeShouldRun; } }

    [SerializeField] private GameObject _fadeToBlack;
    private bool _fadeToBlackStarted;

    private void Start()
    {
        //Starts the timer automatically
        _roundTimeShouldRun = false;
        _uiAnimator = GetComponentInParent<Animator>();

    }

    void Update()
    {
        if (_roundTimeShouldRun) //RoundTimer is running
        {
            RoundTime -= Time.deltaTime;

            if (RoundTime > 1.0f)
            {
                DisplayTime(RoundTime);
            }
            else if (RoundTime > 0.0f)
            {
                _uiAnimator.SetTrigger("TimesUp");
                TimeText.text = "Time's Up!";
                //Debug.Log("Time has ran out!");
            }
            else if(!_fadeToBlackStarted)
            {
                RoundTime = 0.0f;

                GameObject.FindGameObjectWithTag("System").GetComponentInChildren<TotalScore>().UpdateTotalScoreboard();

                PlayerSetup.instance.InteruptAllPlayerStates();
                PlayerSetup.instance.FreezeAllPlayerInput();
                
                _fadeToBlack.GetComponent<Animator>().SetTrigger("SceneTransition");
                _fadeToBlackStarted = true;
            }
            else if (_fadeToBlack.GetComponent<Animator>().GetCurrentAnimatorStateInfo(0).normalizedTime > 1.5f)
            {
                _timerSceneTransition.ContinueOrEndGame();
            }
        }
        else //CountdownTimer is running
        {
            CountdownTime -= Time.deltaTime;

            if (CountdownTime > 1.0f)
            {
                DisplayTime(CountdownTime);
            }
            else if (CountdownTime > 0.0f)
            {
                TimeText.text = "Go!";
                //Debug.Log("Time has ran out!");
            }
            else if(!_roundTimeShouldRun && !_fadeToBlackStarted)
            {
                CountdownTime = 0.0f;
                _roundTimeShouldRun = true;
                PlayerSetup.instance.UnFreezeAllPlayerInput();
            }
        }

    }

    private void DisplayTime(float timeToDisplay)
    {
        float minutes = Mathf.FloorToInt(timeToDisplay / 60);
        float seconds = Mathf.FloorToInt(timeToDisplay % 60);
        TimeText.text = string.Format("{0:00}:{1:00}", minutes, seconds);
    }
}
