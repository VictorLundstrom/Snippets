using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PlayerAnimationBehaviour : MonoBehaviour
{
    [Header("Component References")]
    public Animator animationAnimator;

    // Animation string IDs ////////////////////////////////////////////////
    private int playerJumpAnimationID;
    private int playerXDirectionAnimationID;
    private int playerYDirectionAnimationID;
    private int playerWindupAnimationID;
    private int playerAttackAnimationID;
    private int playerPickupAnimationID;
    private int playerGrabbedAnimationID;
    private int playerGrabAnimationID;
    private int playerDeadAnimationID;
    private int playerElectrocutionAnimationID;
    private int playerRifleGripAnimationID;
    private int playerRifleFireAnimationID;
    private int playerSmashWindupAnimationID;
    private int playerSmashAttackAnimationID;


    private void Awake()
    {
        SetupAnimationIDs();
    }

    void SetupAnimationIDs()
    {
        playerJumpAnimationID = Animator.StringToHash("Jump");
        playerXDirectionAnimationID = Animator.StringToHash("DirX");
        playerYDirectionAnimationID = Animator.StringToHash("DirY");
        playerWindupAnimationID = Animator.StringToHash("Windup");
        playerAttackAnimationID = Animator.StringToHash("Attack");
        playerPickupAnimationID = Animator.StringToHash("Pickup");
        playerGrabbedAnimationID = Animator.StringToHash("Grabbed");
        playerGrabAnimationID = Animator.StringToHash("Grab");
        playerDeadAnimationID = Animator.StringToHash("Dead");
        playerElectrocutionAnimationID = Animator.StringToHash("Electrocution");
        playerRifleGripAnimationID = Animator.StringToHash("RifleGrip");
        playerRifleFireAnimationID = Animator.StringToHash("RifleFire");
        playerSmashWindupAnimationID = Animator.StringToHash("SmashWindup");
        playerSmashAttackAnimationID = Animator.StringToHash("SmashAttack");
    }

    public void UpdateDirectionAnimation(float dirX, float dirY)
    {
        animationAnimator.SetFloat(playerXDirectionAnimationID, dirX);
        animationAnimator.SetFloat(playerYDirectionAnimationID, dirY);
    }

    public void PlayJumpAnimation()
    {
        animationAnimator.SetTrigger(playerJumpAnimationID);
    }

    public void PlayWindupAnimation()
    {
        animationAnimator.SetTrigger(playerWindupAnimationID);
    }
    public void PlayAttackAnimation()
    {
        animationAnimator.SetTrigger(playerAttackAnimationID);
    }

    public void TogglePickupAnimation(bool play)
    {
        animationAnimator.SetBool(playerPickupAnimationID, play);
    }

    public void ToggleGrabAnimation(bool play)
    {
        animationAnimator.SetBool(playerGrabAnimationID, play);
    }
    public void ToggleGrabbedAnimation(bool play)
    {
        animationAnimator.SetBool(playerGrabbedAnimationID, play);
    }
    public void ToggleDeadAnimation(bool play)
    {
        animationAnimator.SetBool(playerDeadAnimationID, play);
    }
    public void ToggleElectrocutionAnimation(bool play)
    {
        animationAnimator.SetBool(playerElectrocutionAnimationID, play);
    }

    public void ToggleRifleGripAnimation(bool play)
    {
        animationAnimator.SetBool(playerRifleGripAnimationID, play);
    }
    public void PlayRifleFireAnimation()
    {
        animationAnimator.SetTrigger(playerRifleFireAnimationID);
    }

    public void PlaySmashWindupAnimation()
    {
        animationAnimator.SetTrigger(playerSmashWindupAnimationID);
    }
    public void PlaySmashAttackAnimation()
    {
        animationAnimator.SetTrigger(playerSmashAttackAnimationID);
    }
}
