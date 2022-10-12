using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class TimerSceneTransition : MonoBehaviour
{
    public void ContinueOrEndGame()
    {
        PlayerSetup.instance.SwitchActionMapForAll("Player");

        PlayerSetup.instance.ResetPickupIndicator(); //Used to Disable PickupIndicator to prevent error
        PlayerSetup.instance.ResetElectrocution(); //Sometimes this VFX/animation gets through scenes, so stop it if its active

        if (LevelManager.Instance.CurrLevel < LevelManager.Instance.NumLevels) //If we should go to Shop
        {
            LevelManager.Instance.LoadShop();
        }
        else //If we should go to Scoreboard
        {
            Destroy(GameObject.FindGameObjectWithTag("EventSystem"));
            LevelManager.Instance.LoadScoreboard();
        }
    }
}
