using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.InputSystem;
using UnityEngine.InputSystem.UI;


public class ScoreboardEventSystem : MonoBehaviour
{
    [SerializeField] private GameObject _scoreboardCanvas;
    [SerializeField] private GameObject _mainmenuButton;
    [SerializeField] private GameObject[] _playerContinueCanvasesButtons;

    private void Awake()
    {
        PlayerSetup.instance.EnablePlayerEventSystemForAll();

        List<GameObject> playerList = PlayerSetup.instance.PlayerList;

        for (int i = 0; i < playerList.Count; i++)
        {
            _playerContinueCanvasesButtons[i].SetActive(true);
            playerList[i].transform.parent.GetChild(2).GetComponent<MultiplayerEventSystem>().playerRoot = _playerContinueCanvasesButtons[i];

            playerList[i].transform.parent.GetChild(2).GetComponent<MultiplayerEventSystem>().SetSelectedGameObject(null);
            playerList[i].transform.parent.GetChild(2).GetComponent<MultiplayerEventSystem>().SetSelectedGameObject(_playerContinueCanvasesButtons[i]);
        }

        _scoreboardCanvas.GetComponent<FinalScoreboard>().MoveContinueButton();
    }
}
