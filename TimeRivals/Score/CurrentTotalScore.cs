using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CurrentTotalScore : MonoBehaviour
{
    private TotalScore _totalScore;

    private void Awake()
    {
        _totalScore = GameObject.FindGameObjectWithTag("System").gameObject.GetComponentInChildren<TotalScore>();

        int _tokens = 10;
        for (int i = 0; i < _totalScore.TotalScoreboard.Length; i++) // Give all players 10 tokens
        {
            _totalScore.TotalScoreboard[i].GetComponent<PlayerController>().Tokens += _tokens;
        }
    }
}
