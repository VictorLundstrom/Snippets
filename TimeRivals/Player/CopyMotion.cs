using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CopyMotion : MonoBehaviour
{
    [SerializeField] private Transform _targetLimb;
    [SerializeField] private ConfigurableJoint _configurableJoint;
    [SerializeField] private Animator _animationAnimator;
    [SerializeField] private PlayerMovementBehaviour _playerMovementBehaviour;
    [SerializeField] private HealthSystem _healthSystem;


    [SerializeField] private bool _usedInMovement;
    [SerializeField] private bool _usedInJump;
    [SerializeField] private bool _usedInAttack;
    [SerializeField] private bool _usedInPickup;
    [SerializeField] private bool _usedInGrab;
    [SerializeField] private bool _usedInRifleGrip;
    [SerializeField] private bool _usedInElectrocution;

    private PlayerStates _animToCopy = PlayerStates.UNSET;
    private bool _shouldCopyAnim;

    void Update()
    {
        if (_playerMovementBehaviour.PlayerState != _animToCopy) //if our _animToCopy is NOT up to date
        {
            _shouldCopyAnim = false; //Reset variable from last time, since we don't know if we should copy this new animation

            _animToCopy = _playerMovementBehaviour.PlayerState;

            switch (_animToCopy)
            {
                case PlayerStates.MOVEMENT:
                    if (_usedInMovement)
                        _shouldCopyAnim = true;
                    else if (_usedInRifleGrip && (_playerMovementBehaviour.CurrAttack == AttackIndexes.RIFLE || _playerMovementBehaviour.CurrAttack == AttackIndexes.CROSSBOW))
                        _shouldCopyAnim = true;
                        break;
                case PlayerStates.JUMP:
                    if (_usedInJump)
                        _shouldCopyAnim = true;
                    else if (_usedInRifleGrip && (_playerMovementBehaviour.CurrAttack == AttackIndexes.RIFLE || _playerMovementBehaviour.CurrAttack == AttackIndexes.CROSSBOW))
                        _shouldCopyAnim = true;
                    break;
                case PlayerStates.ATTACK:
                    if (_usedInAttack)
                        _shouldCopyAnim = true;
                    break;
                case PlayerStates.PICKUP:
                    if (_usedInPickup)
                        _shouldCopyAnim = true;
                    break;
                case PlayerStates.GRAB:
                    if (_usedInGrab)
                        _shouldCopyAnim = true;
                    break;
                case PlayerStates.GRABBED: //Don't copy anmiation
                    break;
                case PlayerStates.DEAD: //Don't copy animation
                    if (_healthSystem.Electric.isPlaying && _usedInElectrocution)
                        _shouldCopyAnim = true;
                    break;
                default:
                    Debug.Log("Error at CopyMotion SwitchCase");
                    break;
            }
        }

        if (_shouldCopyAnim)
            _configurableJoint.transform.rotation = _targetLimb.rotation;
        

    } //Update()

} //CopyMotion class
