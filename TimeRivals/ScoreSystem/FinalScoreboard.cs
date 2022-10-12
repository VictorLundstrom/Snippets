using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using TMPro;

public class FinalScoreboard : MonoBehaviour
{
    private TotalScore _totalScore;

    [SerializeField] private TextMeshProUGUI _P1_Score;
    [SerializeField] private TextMeshProUGUI _P2_Score;
    [SerializeField] private TextMeshProUGUI _P3_Score;
    [SerializeField] private TextMeshProUGUI _P4_Score;

    [SerializeField] private GameObject _P1_Container;
    [SerializeField] private GameObject _P2_Container;
    [SerializeField] private GameObject _P3_Container;
    [SerializeField] private GameObject _P4_Container;

    private string _pointsPrefix = " | Kills: ";


    private void Awake()
    {
        _totalScore = GameObject.FindGameObjectWithTag("System").gameObject.GetComponentInChildren<TotalScore>();

        AddText();
    }

    private void AddText()
    {
        List<GameObject> playerlistRef = PlayerSetup.instance.PlayerList;
        if (LevelManager.Instance.PlayMode == (int)PlayModes.TWO_PLAYER) //if 2 players
        {
            _P1_Container.SetActive(true);
            _P1_Score.text = _pointsPrefix + playerlistRef[0].GetComponent<PlayerController>().TotalPoints;
            _P2_Container.SetActive(true);
            _P2_Score.text = _pointsPrefix + playerlistRef[1].GetComponent<PlayerController>().TotalPoints;
        }
        else if (LevelManager.Instance.PlayMode == (int)PlayModes.THREE_PLAYER) // if 3 players
        {
            _P1_Container.SetActive(true);
            _P1_Score.text = _pointsPrefix + playerlistRef[0].GetComponent<PlayerController>().TotalPoints;
            _P2_Container.SetActive(true);
            _P2_Score.text = _pointsPrefix + playerlistRef[1].GetComponent<PlayerController>().TotalPoints;
            _P3_Container.SetActive(true);
            _P3_Score.text = _pointsPrefix + playerlistRef[2].GetComponent<PlayerController>().TotalPoints;

        }
        else if (LevelManager.Instance.PlayMode == (int)PlayModes.FOUR_PLAYER) // if 4 players
        {
            _P1_Container.SetActive(true);
            _P1_Score.text = _pointsPrefix + playerlistRef[0].GetComponent<PlayerController>().TotalPoints;
            _P2_Container.SetActive(true);
            _P2_Score.text = _pointsPrefix + playerlistRef[1].GetComponent<PlayerController>().TotalPoints;
            _P3_Container.SetActive(true);
            _P3_Score.text = _pointsPrefix + playerlistRef[2].GetComponent<PlayerController>().TotalPoints;
            _P4_Container.SetActive(true);
            _P4_Score.text = _pointsPrefix + playerlistRef[3].GetComponent<PlayerController>().TotalPoints;
        }
    }

    public void MoveContinueButton()
    {
        List<GameObject> playerlistRef = PlayerSetup.instance.PlayerList;

        if (LevelManager.Instance.PlayMode == (int)PlayModes.TWO_PLAYER) //if 2 players
        {
            _P1_Container.transform.SetSiblingIndex(playerlistRef[0].GetComponent<PlayerController>().TotalRank);
            _P2_Container.transform.SetSiblingIndex(playerlistRef[1].GetComponent<PlayerController>().TotalRank);
        }

        if (LevelManager.Instance.PlayMode == (int)PlayModes.THREE_PLAYER) //if 3 players
        {
            _P1_Container.transform.SetSiblingIndex(playerlistRef[0].GetComponent<PlayerController>().TotalRank);
            _P2_Container.transform.SetSiblingIndex(playerlistRef[1].GetComponent<PlayerController>().TotalRank);
            _P3_Container.transform.SetSiblingIndex(playerlistRef[2].GetComponent<PlayerController>().TotalRank);
        }

        if (LevelManager.Instance.PlayMode == (int)PlayModes.FOUR_PLAYER) //if 4 players
        {
            _P1_Container.transform.SetSiblingIndex(playerlistRef[0].GetComponent<PlayerController>().TotalRank);
            _P2_Container.transform.SetSiblingIndex(playerlistRef[1].GetComponent<PlayerController>().TotalRank);
            _P3_Container.transform.SetSiblingIndex(playerlistRef[2].GetComponent<PlayerController>().TotalRank);
            _P4_Container.transform.SetSiblingIndex(playerlistRef[3].GetComponent<PlayerController>().TotalRank);
        }

    }
}
